#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ctype.h>
#include "html_page.h"
#include "config.h"

// ================= 配置区 =================
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 30000;

// ZZK 的目标 UUID
static BLEUUID serviceUUID("2d220000-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID charUUID("2d220001-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID batteryCharUUID("2d220002-516b-47bd-a33b-2c93889ac9b7"); // 新增电池特征值

// ================= 全局对象 =================
AsyncWebServer server(80);
Preferences preferences; // Flash 记忆体对象
AsyncEventSource events("/events"); // 新增 SSE 事件源，用于向网页实时推送电量

String targetMacAddress = ""; // 当前绑定的目标 MAC 地址

// BLE 状态标志
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false; // 用于断线重连扫描控制，只有绑定设备后才开启

// 特征值指针与设备指针
static BLERemoteCharacteristic* pRemoteCharacteristic; // 开关控制指针
static BLERemoteCharacteristic* pBatteryCharacteristic; // 电池电量指针
static BLEAdvertisedDevice* myDevice;
static BLEClient* pClient = nullptr;

// Web 与 BLE 之间的通信桥梁（-1 代表无动作，0 关灯，1 开灯）
volatile int pendingAction = -1;
// 添加一个专门存电量的变量
volatile int zzkBattery = -1; // -1 表示尚未获取到有效电量

// ================= BLE 客户端连接状态回调 =================
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.println(">>> 物理层已连接到 ZZK 开关！");
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    pRemoteCharacteristic = nullptr;
    pBatteryCharacteristic = nullptr;
    Serial.println("<<< 与 ZZK 断开连接，准备重新扫描...");
  }
};

static MyClientCallback clientCallback;

void disconnectCurrentDevice() {
    if (pClient != nullptr && pClient->isConnected()) {
        pClient->disconnect();
    }
    connected = false;
    pRemoteCharacteristic = nullptr;
    pBatteryCharacteristic = nullptr;
    zzkBattery = -1;
}

bool isValidMacAddress(String mac) {
    mac.trim();
    if (mac.length() != 17) {
        return false;
    }

    for (int i = 0; i < 17; i++) {
        char c = mac.charAt(i);
        if ((i + 1) % 3 == 0) {
            if (c != ':') {
                return false;
            }
        } else if (!isxdigit(c)) {
            return false;
        }
    }

    return true;
}

// ================= BLE 电池 Notify 回调函数 =================
// 当墙上的 ZZK 电量发生变化并推送时，会瞬间触发这里
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (length > 0) {
        uint8_t batteryLevel = pData[0];

        // 更新全局电量变量，给上面的 /battery 路由备用
        zzkBattery = batteryLevel;

        Serial.printf("🔋 收到 ZZK 电量推送: %d%%\n", zzkBattery);

        // 将电量数字转换为字符串，并通过 SSE 推送给所有连着网关的手机网页
        events.send(String(batteryLevel).c_str(), "battery", millis());
    }
}

// ================= BLE 雷达扫描回调 (平时重连用) =================
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // 当扫描到任何蓝牙设备时，会触发此回调
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // 检查该设备是否广播了我们专属的 ZZK 服务 UUID
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      String scannedMac = advertisedDevice.getAddress().toString().c_str();
      scannedMac.toLowerCase();
      String boundMac = targetMacAddress;
      boundMac.toLowerCase();

      if (boundMac.length() == 0 || scannedMac != boundMac) {
        return;
      }

      BLEDevice::getScan()->stop(); // 扫到专属目标！立刻停止雷达扫描以节省资源

      // 保存找到的设备信息，留给主循环去发起连接
      if (myDevice != nullptr) {
        delete myDevice;
        myDevice = nullptr;
      }
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true; // 允许后续的断线重连逻辑
      Serial.printf("🎯 成功锁定目标 ZZK 设备 MAC: %s \n", advertisedDevice.getAddress().toString().c_str());
    }
  }
};

// ================= Web 服务器初始化 =================
void setupWebServer() {
    // 1. 根目录：吐出 HTML 前端主网页
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // 2. 接收控制开关动作指令的 API 接口
    server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("action")) {
            String action = request->getParam("action")->value();
            if (action != "0" && action != "1") {
                request->send(400, "text/plain", "Invalid action");
                return;
            }
            pendingAction = action.toInt(); // 赋值给全局变量，交给主循环处理
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Bad Request");
        }
    });

    // 3. 核心 API：触发雷达扫描并返回 JSON 列表
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("📡 网页触发周边 ZZK 设备扫描...");

    BLEScan* pBLEScan = BLEDevice::getScan();
    //【修复3】强制停止后台可能正在运行的扫描，防止撞车
    pBLEScan->stop();
    pBLEScan->clearResults();

    // 阻塞式扫描 3 秒钟（由于是配置阶段，短时间阻塞可接受）
    BLEScanResults foundDevices = pBLEScan->start(3, false);

    JsonDocument doc; // 创建 JSON 文档
    JsonArray array = doc.to<JsonArray>();

    for(int i = 0; i < foundDevices.getCount(); i++) {
      BLEAdvertisedDevice dev = foundDevices.getDevice(i);

        // 1. 严格检查真实名字匹配
        bool nameMatch = false;
        if (dev.haveName()) {
          String actualName = dev.getName().c_str();
          actualName.toUpperCase(); // 转为大写比对，防止大小写干扰
          if (actualName.indexOf("ZZK") >= 0) {
            nameMatch = true;
          }
        }

        // 2. 严格检查底层协议 UUID 匹配
        bool uuidMatch = (dev.haveServiceUUID() && dev.isAdvertisingService(serviceUUID));

        // 3. 只有真实名字匹配，或者 UUID 匹配，才允许放行！
        if (nameMatch || uuidMatch) {
          JsonObject obj = array.add<JsonObject>();
          // 发给网页时，如果有真名就用真名，没真名说明它是靠 UUID 匹配进来的设备
          obj["name"] = dev.haveName() ? dev.getName().c_str() : "ZZK_Switch (未命名)";
          obj["mac"] = dev.getAddress().toString().c_str();
          obj["rssi"] = dev.getRSSI();
        }
    }

        pBLEScan->clearResults(); // 释放内存
    String response;
    serializeJson(doc, response); // 打包成 JSON 字符串
    request->send(200, "application/json", response);
  });

    // 4. 核心 API：接收网页选中的 MAC 地址并永久绑定
    server.on("/bind", HTTP_GET, [](AsyncWebServerRequest *request){
      if(request->hasParam("mac")) {
        String mac = request->getParam("mac")->value();
        mac.trim();
        if (!isValidMacAddress(mac)) {
          request->send(400, "text/plain", "Invalid MAC");
          return;
        }

        Serial.printf("🔗 网页请求绑定新设备 MAC: %s\n", mac.c_str());

        // 写入内部 Flash 永久记忆
        preferences.putString("target_mac", mac);
        targetMacAddress = mac; // 更新当前运行时的目标

        // 如果当前连着旧设备，立刻断开
        disconnectCurrentDevice();
        doScan = true;

        request->send(200, "text/plain", "Bind Success");
      } else {
        request->send(400, "text/plain", "Missing MAC");
      }
    });

    server.on("/battery", HTTP_GET, [](AsyncWebServerRequest *request){
        if (zzkBattery != -1) {
            request->send(200, "text/plain", String(zzkBattery));
        } else {
            request->send(200, "text/plain", "Waiting...");
        }
    });
}

// ================= 深入连接并获取特征值逻辑 =================
bool connectToServer() {
    Serial.print("🔄 正在建立逻辑连接... ");
    if (myDevice == nullptr) {
        Serial.println("没有可连接的目标设备。");
        return false;
    }

    if (pClient == nullptr) {
        pClient = BLEDevice::createClient();
        pClient->setClientCallbacks(&clientCallback);
    } else if (pClient->isConnected()) {
        pClient->disconnect();
    }

    if (!pClient->connect(myDevice)) {
        Serial.println("连接失败！");
        return false;
    }

    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("❌ 找不到目标服务 UUID，设备可能被串改！");
        pClient->disconnect();
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("❌ 找不到目标特征值 UUID！");
        pClient->disconnect();
        return false;
    }

    pBatteryCharacteristic = pRemoteService->getCharacteristic(batteryCharUUID);
    if (pBatteryCharacteristic != nullptr) {
        if (pBatteryCharacteristic->canNotify()) {
            pBatteryCharacteristic->registerForNotify(notifyCallback);
            Serial.println("✅ 电池监听通道就绪！");
        }
        if (pBatteryCharacteristic->canRead()) {
            uint8_t currentBat = pBatteryCharacteristic->readUInt8();
            zzkBattery = currentBat;
            events.send(String(currentBat).c_str(), "battery", millis());
        }
    }

    connected = true;
    return true;
}

// ================= 系统初始化 =================
void setup() {
    server.addHandler(&events);
    Serial.begin(115200);
    Serial.println("🚀 CCJ Gateway 启动中...");

    // 1. 初始化 Flash 记忆体并读取绑定的 MAC
    preferences.begin("zzk-app", false);
    targetMacAddress = preferences.getString("target_mac", "");

    if(targetMacAddress == "") {
        Serial.println("⚠️ 当前未绑定任何 ZZK 设备，请通过网页扫描绑定！");
        doScan = false;
    } else {
        Serial.printf("🎯 从记忆中读取到绑定的目标 MAC: %s\n", targetMacAddress.c_str());
        doScan = true;
    }

    // 2. 连接 WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("正在连接 WiFi");
    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500); Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi 连接成功！请在浏览器中访问以下 IP 地址：");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n⚠️ WiFi 连接超时，已启动临时 AP：CCJ-Gateway-Setup");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("CCJ-Gateway-Setup");
        Serial.println(WiFi.softAPIP());
    }

    // 3. 初始化蓝牙引擎并开启雷达扫描
    BLEDevice::init("CCJ_Gateway_Central");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    // 4. 启动异步网页服务器
    setupWebServer();
    server.begin();
}

// ================= 主循环 =================
void loop() {
    // 1. 如果扫描回调函数找到了设备，这里会接管并执行真正的连接动作
    if (doConnect) {
        if (!connectToServer()) {
            Serial.println("连接流程异常终止，准备重新扫描...");
        }
        if (myDevice != nullptr) {
            delete myDevice;
            myDevice = nullptr;
        }
        doConnect = false;
    }

    // 2. 断线自动重连保活机制
    if (!connected && doScan) {
        BLEDevice::getScan()->start(5, false);
    }

    // 3. 跨线程安全处理：网页发来了按键动作
    if (pendingAction != -1 && connected) {
        uint8_t valueToSend = (pendingAction == 1) ? 0x01 : 0x00;
        // 必须确保蓝牙底层处于已连接状态，且指针未悬空
        if (connected && pRemoteCharacteristic != nullptr) {
            pRemoteCharacteristic->writeValue(&valueToSend, 1);
            Serial.printf("⚡ 成功向 ZZK 发射指令包: 0x%02X\n", valueToSend);
        } else {
            Serial.println("⚠️ 蓝牙断开或未就绪，指令丢弃！");
        }

        // 动作执行完毕，清空标志位
        pendingAction = -1;
    }

    delay(50); // 喂狗，避免 RTOS 任务榨干 CPU
}
