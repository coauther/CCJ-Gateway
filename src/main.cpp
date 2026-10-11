#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ctype.h>
#include <mutex>
#include "html_page.h"
#include "config.h"
#include "ota_maintenance.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 30000;

static BLEUUID serviceUUID("2d220000-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID charUUID("2d220001-516b-47bd-a33b-2c93889ac9b7");
static BLEUUID batteryCharUUID("2d220002-516b-47bd-a33b-2c93889ac9b7");

AsyncWebServer server(80);
Preferences preferences;
AsyncEventSource events("/events");

struct GatewayState {
    String targetMacAddress;
    bool connected = false; // True only after the control characteristic is ready.
    int battery = -1;
    int pendingAction = -1;
    OtaMaintenance maintenance;
};

static GatewayState state;
static std::mutex stateMutex;
// Serialize scans, binding and GATT operations. Callbacks only take stateMutex.
// Never hold stateMutex while calling blocking BLE APIs: they wait for callbacks.
static std::mutex bleMutex;
static BLEClient* pClient = nullptr;
static BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

GatewayState snapshot() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return state;
}

bool bleBlocked() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return state.maintenance.blocksBle();
}

void clearConnectionState() {
    std::lock_guard<std::mutex> lock(stateMutex);
    state.connected = false;
    state.battery = -1;
    state.pendingAction = -1;
}

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient*) override {
        Serial.println(">>> BLE 已连接，正在发现服务...");
    }
    void onDisconnect(BLEClient*) override {
        clearConnectionState();
        events.send("-1", "battery", millis());
        Serial.println("<<< BLE 已断开；维护期间暂停重连。");
    }
};
static MyClientCallback clientCallback;

// Caller owns bleMutex. Characteristic pointers are not changed in callbacks.
void disconnectCurrentDevice() {
    clearConnectionState();
    pRemoteCharacteristic = nullptr;
    if (pClient != nullptr && pClient->isConnected()) pClient->disconnect();
    events.send("-1", "battery", millis());
}

bool isValidMacAddress(const String& mac) {
    if (mac.length() != 17) return false;
    for (int i = 0; i < 17; ++i) {
        if ((i + 1) % 3 == 0) {
            if (mac[i] != ':') return false;
        } else if (!isxdigit(static_cast<unsigned char>(mac[i]))) {
            return false;
        }
    }
    return true;
}

void updateBattery(uint8_t level) {
    if (level > 100) return;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (state.maintenance.blocksBle()) return;
        state.battery = level;
    }
    events.send(String(level).c_str(), "battery", millis());
}

static void notifyCallback(BLERemoteCharacteristic*, uint8_t* data, size_t length, bool) {
    if (length > 0) updateBattery(data[0]);
}

void sendStatus(AsyncWebServerRequest* request, int code = 200) {
    const GatewayState current = snapshot();
    JsonDocument doc;
    doc["bound_mac"] = current.targetMacAddress;
    doc["connected"] = current.connected;
    doc["battery"] = current.battery;
    doc["maintenance"] = current.maintenance.blocksBle();
    doc["maintenance_state"] = current.maintenance.name();
    doc["remaining_seconds"] = current.maintenance.remainingSeconds(millis());
    String body;
    serializeJson(doc, body);
    AsyncWebServerResponse* response = request->beginResponse(code, "application/json", body);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
}

void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", index_html);
    });
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        sendStatus(request);
    });

    // The loop confirms the disconnection before starting the 180-second timer.
    server.on("/maintenance", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("action", true)) {
            request->send(400, "text/plain", "Missing action");
            return;
        }
        const String action = request->getParam("action", true)->value();
        if (action != "release" && action != "resume") {
            request->send(400, "text/plain", "Invalid action");
            return;
        }
        bool hasTarget;
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            hasTarget = state.targetMacAddress.length() > 0;
            if (hasTarget) {
                state.pendingAction = -1;
                if (action == "release") state.maintenance.requestRelease();
                else state.maintenance.requestResume();
            }
        }
        if (!hasTarget) {
            request->send(409, "text/plain", "No bound switch");
            return;
        }
        sendStatus(request, 202);
    });

    server.on("/control", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("action")) {
            request->send(400, "text/plain", "Missing action");
            return;
        }
        const String action = request->getParam("action")->value();
        if (action != "0" && action != "1") {
            request->send(400, "text/plain", "Invalid action");
            return;
        }
        int code = 200;
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            if (state.maintenance.blocksBle()) code = 409;
            else if (!state.connected) code = 503;
            else state.pendingAction = action.toInt();
        }
        request->send(code, "text/plain", code == 200 ? "OK" :
                      code == 409 ? "OTA maintenance active" : "Switch disconnected");
    });

    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* request) {
        std::unique_lock<std::mutex> bleLock(bleMutex, std::try_to_lock);
        if (!bleLock.owns_lock() || bleBlocked()) {
            request->send(409, "text/plain", "BLE busy or OTA maintenance active");
            return;
        }
        BLEScan* scan = BLEDevice::getScan();
        scan->clearResults();
        BLEScanResults foundDevices = scan->start(3, false);
        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();
        for (int i = 0; i < foundDevices.getCount(); ++i) {
            BLEAdvertisedDevice dev = foundDevices.getDevice(i);
            String name = dev.haveName() ? dev.getName().c_str() : "";
            name.toUpperCase();
            if (name.indexOf("ZZK") >= 0 ||
                (dev.haveServiceUUID() && dev.isAdvertisingService(serviceUUID))) {
                JsonObject obj = array.add<JsonObject>();
                obj["name"] = dev.haveName() ? dev.getName().c_str() : "ZZK_Switch (未命名)";
                obj["mac"] = dev.getAddress().toString().c_str();
                obj["rssi"] = dev.getRSSI();
            }
        }
        scan->clearResults();
        String body;
        serializeJson(doc, body);
        request->send(200, "application/json", body);
    });

    server.on("/bind", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("mac")) {
            request->send(400, "text/plain", "Missing MAC");
            return;
        }
        String mac = request->getParam("mac")->value();
        mac.trim();
        mac.toLowerCase();
        if (!isValidMacAddress(mac)) {
            request->send(400, "text/plain", "Invalid MAC");
            return;
        }
        std::unique_lock<std::mutex> bleLock(bleMutex, std::try_to_lock);
        if (!bleLock.owns_lock() || bleBlocked()) {
            request->send(409, "text/plain", "BLE busy or OTA maintenance active");
            return;
        }
        if (preferences.putString("target_mac", mac) == 0) {
            request->send(500, "text/plain", "Could not save binding");
            return;
        }
        disconnectCurrentDevice();
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state.targetMacAddress = mac;
        }
        request->send(200, "text/plain", "Bind Success");
    });

    server.on("/battery", HTTP_GET, [](AsyncWebServerRequest* request) {
        const int battery = snapshot().battery;
        request->send(200, "text/plain", battery < 0 ? "Waiting..." : String(battery));
    });
}

// Caller owns bleMutex. Recheck maintenance between blocking GATT operations:
// a release requested while connecting must also disconnect that late connection.
bool connectToServer(BLEAdvertisedDevice* device) {
    if (bleBlocked()) return false;
    if (pClient == nullptr) {
        pClient = BLEDevice::createClient();
        pClient->setClientCallbacks(&clientCallback);
    }
    if (!pClient->connect(device)) return false;
    if (bleBlocked()) { disconnectCurrentDevice(); return false; }

    // Firmware updates can move GATT handles. Reusing this BLEClient must not
    // reuse service/characteristic objects from the previous firmware.
    pRemoteCharacteristic = nullptr;
    pClient->getServices();
    if (bleBlocked() || !pClient->isConnected()) { disconnectCurrentDevice(); return false; }
    BLERemoteService* service = pClient->getService(serviceUUID);
    if (service == nullptr || bleBlocked()) { disconnectCurrentDevice(); return false; }
    pRemoteCharacteristic = service->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr || !pRemoteCharacteristic->canWrite() || bleBlocked()) {
        disconnectCurrentDevice();
        return false;
    }
    BLERemoteCharacteristic* battery = service->getCharacteristic(batteryCharUUID);
    if (battery != nullptr && !bleBlocked()) {
        if (battery->canNotify()) battery->registerForNotify(notifyCallback);
        if (battery->canRead() && !bleBlocked() && pClient->isConnected()) {
            updateBattery(battery->readUInt8());
        }
    }
    bool ready;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        ready = !state.maintenance.blocksBle() && pClient->isConnected();
        state.connected = ready;
    }
    if (!ready) disconnectCurrentDevice();
    return ready;
}

// No scan callback queues connections. Results belong to the current scan only,
// and are discarded if maintenance was requested while scanning.
void scanForBoundDevice(const String& targetMac) {
    BLEScan* scan = BLEDevice::getScan();
    scan->clearResults();
    BLEScanResults devices = scan->start(2, false);
    if (!bleBlocked()) {
        for (int i = 0; i < devices.getCount(); ++i) {
            BLEAdvertisedDevice device = devices.getDevice(i);
            String mac = device.getAddress().toString().c_str();
            if (mac.equalsIgnoreCase(targetMac) && device.haveServiceUUID() &&
                device.isAdvertisingService(serviceUUID)) {
                connectToServer(&device);
                break;
            }
        }
    }
    scan->clearResults();
}

void processBle() {
    std::unique_lock<std::mutex> bleLock(bleMutex, std::try_to_lock);
    if (!bleLock.owns_lock()) return;
    OtaMaintenance::Phase phase;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        state.maintenance.tick(millis());
        phase = state.maintenance.phase();
    }
    if (phase == OtaMaintenance::Phase::Releasing) {
        BLEDevice::getScan()->stop();
        BLEDevice::getScan()->clearResults();
        disconnectCurrentDevice();
        // disconnect() requests an asynchronous disconnect. Do not announce
        // the OTA window until the BLE client confirms that the link is gone.
        if (pClient == nullptr || !pClient->isConnected()) {
            std::lock_guard<std::mutex> lock(stateMutex);
            state.maintenance.released(millis());
        }
        return;
    }
    if (phase == OtaMaintenance::Phase::Resuming) {
        std::lock_guard<std::mutex> lock(stateMutex);
        state.pendingAction = -1;
        state.maintenance.resumed();
    }
    const GatewayState current = snapshot();
    if (current.maintenance.blocksBle()) return;
    if (!current.connected && current.targetMacAddress.length() > 0) {
        // A resume can overtake an asynchronous disconnect. Finish releasing
        // the previous link before attempting another connection.
        if (pClient != nullptr && pClient->isConnected()) {
            disconnectCurrentDevice();
            return;
        }
        scanForBoundDevice(current.targetMacAddress);
        return;
    }
    int action = -1;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!state.maintenance.blocksBle() && state.connected) action = state.pendingAction;
        state.pendingAction = -1;
    }
    if (action != -1 && !bleBlocked() && pClient != nullptr &&
        pClient->isConnected() && pRemoteCharacteristic != nullptr) {
        uint8_t value = action == 1 ? 0x01 : 0x00;
        pRemoteCharacteristic->writeValue(&value, 1, true);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("CCJ Gateway 启动中...");
    preferences.begin("zzk-app", false);
    state.targetMacAddress = preferences.getString("target_mac", "");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    const unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi 已连接，网关地址：");
        Serial.println(WiFi.localIP());
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("CCJ-Gateway-Setup");
        Serial.println("\nWiFi 超时，已启动 CCJ-Gateway-Setup：");
        Serial.println(WiFi.softAPIP());
    }

    BLEDevice::init("CCJ_Gateway_Central");
    BLEScan* scan = BLEDevice::getScan();
    scan->setActiveScan(true); // ZZK-SA2-XXXX names are in the scan response.
    scan->setInterval(100);
    scan->setWindow(99);
    server.addHandler(&events);
    setupWebServer();
    server.begin();
}

void loop() {
    processBle();
    delay(50);
}
