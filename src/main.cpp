#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include "html_page.h"
#include "config.h"

// ================= 配置区 =================
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ZZK 的目标 UUID
static BLEUUID serviceUUID("2d220000-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID charUUID("2d220001-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID batteryCharUUID("2d220002-516b-47bd-a33b-2c93889ac9b7"); // 新增电池特征值

// ================= 全局对象 =================
AsyncWebServer server(80);
AsyncEventSource events("/events"); // 新增 SSE 事件源，用于向网页实时推送电量

// BLE 状态标志
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = true; // 用于断线重连扫描控制

// 特征值指针与设备指针
static BLERemoteCharacteristic* pRemoteCharacteristic; // 开关控制指针
static BLERemoteCharacteristic* pBatteryCharacteristic; // 电池电量指针
static BLEAdvertisedDevice* myDevice;

// Web 与 BLE 之间的通信桥梁（-1 代表无动作，0 关灯，1 开灯）
volatile int pendingAction = -1;

// ================= BLE 客户端连接状态回调 =================
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.println(">>> 物理层已连接到 ZZK 开关！");
  }
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("<<< 与 ZZK 断开连接，准备重新扫描...");
  }
};

// ================= BLE 电池 Notify 回调函数 =================
// 当墙上的 ZZK 电量发生变化并推送时，会瞬间触发这里
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (length > 0) {
        uint8_t batteryLevel = pData[0];
        Serial.printf("🔋 收到 ZZK 电量推送: %d%%\n", batteryLevel);

        // 将电量数字转换为字符串，并通过 SSE 推送给所有连着网关的手机网页
        events.send(String(batteryLevel).c_str(), "battery", millis());
    }
}

// ================= BLE 雷达扫描回调 =================
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // 当扫描到任何蓝牙设备时，会触发此回调
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // 检查该设备是否广播了我们专属的 ZZK 服务 UUID
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop(); // 命中目标！立刻停止雷达扫描以节省资源

      // 保存找到的设备信息，留给主循环去发起连接
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true; // 允许后续的断线重连逻辑
      Serial.printf("🎯 成功锁定目标 ZZK 设备 MAC: %s \n", advertisedDevice.getAddress().toString().c_str());
    }
  }
};

// ================= Web 服务器初始化 =================
void setupWebServer() {
    // 根目录：吐出 HTML 页面
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    // 接收控制指令的 API 接口
    server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("action")) {
            String action = request->getParam("action")->value();
            pendingAction = action.toInt(); // 赋值给全局变量，交给主循环处理
            request->send(200, "text/plain", "OK");
        } else {
            request->send(400, "text/plain", "Bad Request");
        }
    });

    // 🚀 核心新增：将事件流挂载到异步服务器上
    server.addHandler(&events);

    server.begin();
    Serial.println("🌐 异步 Web 服务器已在 80 端口启动，SSE 事件流准备就绪");
}

// ================= 深入连接并获取特征值逻辑 =================
bool connectToServer() {
    Serial.print("🔄 正在建立逻辑连接... ");
    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());

    // 1. 发起实际的蓝牙连接
    if (!pClient->connect(myDevice)) {
        Serial.println("连接失败！");
        return false;
    }

    // 2. 潜入“文件柜”，寻找指定的“服务（Service）”
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("❌ 找不到目标服务 UUID，设备可能被串改！");
        pClient->disconnect();
        return false;
    }

    // 3. 翻开“抽屉”，寻找控制开灯的“表格（Characteristic）”
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("❌ 找不到目标特征值 UUID！");
        pClient->disconnect();
        return false;
    }

    // 4. 🚀 核心新增：获取并绑定“电池”特征值
    pBatteryCharacteristic = pRemoteService->getCharacteristic(batteryCharUUID);
    if (pBatteryCharacteristic != nullptr) {
        // 如果具有 Notify 权限，则注册上面写好的回调函数
        if (pBatteryCharacteristic->canNotify()) {
            pBatteryCharacteristic->registerForNotify(notifyCallback);
            Serial.println("✅ 电池监听通道就绪！");
        }
        // 为了防止网页刚打开时没数据，主动读取一次当前电量并推送
        if (pBatteryCharacteristic->canRead()) {
            uint8_t currentBat = pBatteryCharacteristic->readUInt8();
            events.send(String(currentBat).c_str(), "battery", millis());
        }
    }

    connected = true;
    return true;
}

// ================= 系统初始化 =================
void setup() {
    Serial.begin(115200);
    Serial.println("🚀 CCJ Gateway 启动中...");

    // 1. 连接 WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("正在连接 WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\n✅ WiFi 连接成功！请在浏览器中访问以下 IP 地址：");
    Serial.println(WiFi.localIP());

    // 2. 启动异步网页服务器
    setupWebServer();

    // 3. 初始化蓝牙引擎并开启雷达扫描
    BLEDevice::init("CCJ_Gateway_Central");

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    pBLEScan->setActiveScan(true);
    Serial.println("📡 开启全频段扫描，寻找 ZZK 开关...");
    pBLEScan->start(5, false); // 扫描持续 5 秒
}

// ================= 主循环 =================
void loop() {
    // 1. 如果扫描回调函数找到了设备，这里会接管并执行真正的连接动作
    if (doConnect) {
        if (!connectToServer()) {
            Serial.println("连接流程异常终止，准备重新扫描...");
        }
        doConnect = false;
    }

    // 2. 断线自动重连保活机制
    if (!connected && doScan) {
        BLEDevice::getScan()->start(5, false);
    }

    // 3. 跨线程安全处理：网页发来了按键动作
    if (pendingAction != -1) {
        uint8_t valueToSend = (uint8_t)pendingAction;

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

    delay(20); // 喂狗，避免 RTOS 任务榨干 CPU
}