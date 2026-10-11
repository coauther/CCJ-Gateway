#pragma once
// Host substitutes for hardware and HTTP transport. Tests compile the actual
// gateway routes/loop, injecting requests during blocking scan/connect calls.
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#define PROGMEM
#define WIFI_STA 1
#define WIFI_AP 2
#define WL_CONNECTED 3
#define WIFI_SSID "test"
#define WIFI_PASSWORD "test"

class String : public std::string {
public:
    using std::string::string;
    String() = default;
    String(const std::string& value) : std::string(value) {}
    String(int value) : std::string(std::to_string(value)) {}
    void trim() {
        const size_t first = find_first_not_of(" \t\r\n");
        *this = first == npos ? "" : substr(first, find_last_not_of(" \t\r\n") - first + 1);
    }
    void toLowerCase() { std::transform(begin(), end(), begin(), ::tolower); }
    void toUpperCase() { std::transform(begin(), end(), begin(), ::toupper); }
    int toInt() const { return std::stoi(*this); }
    int indexOf(const char* part) const { return find(part) == npos ? -1 : int(find(part)); }
    bool equalsIgnoreCase(String other) const {
        String copy = *this; copy.toLowerCase(); other.toLowerCase(); return copy == other;
    }
};
static uint32_t fakeNow = 0;
uint32_t millis() { return fakeNow; }
void delay(uint32_t ms) { fakeNow += ms; }
struct FakeSerial { void begin(int) {} template<class T> void print(T) {} template<class T> void println(T) {} } Serial;
struct FakeWiFi {
    void mode(int) {} void begin(const char*, const char*) {} int status() { return WL_CONNECTED; }
    const char* localIP() { return "127.0.0.1"; } const char* softAPIP() { return localIP(); }
    void softAP(const char*) {}
} WiFi;
class Preferences {
public:
    std::map<std::string, String> values;
    bool begin(const char*, bool) { return true; }
    String getString(const char* key, const char* fallback) { return values.count(key) ? values[key] : fallback; }
    size_t putString(const char* key, const String& value) { values[key] = value; return value.size(); }
};
struct JsonValue { template<class T> void operator=(const T&) {} };
struct JsonObject { JsonValue operator[](const char*) { return {}; } };
struct JsonArray { template<class T> T add() { return {}; } };
struct JsonDocument : JsonObject { template<class T> T to() { return {}; } };
void serializeJson(const JsonDocument&, String& output) { output = "{}"; }
enum { HTTP_GET, HTTP_POST };
struct AsyncWebParameter { String text; String value() { return text; } };
struct AsyncWebServerResponse {
    int code; String body; std::map<std::string, String> headers;
    void addHeader(const char* key, const char* value) { headers[key] = value; }
};
class AsyncWebServerRequest {
public:
    std::map<std::string, AsyncWebParameter> query, form;
    int code = 0; String body; std::map<std::string, String> headers;
    bool hasParam(const char* key, bool post = false) { return (post ? form : query).count(key); }
    AsyncWebParameter* getParam(const char* key, bool post = false) { return &(post ? form : query)[key]; }
    void send(int status, const char*, String value) { code = status; body = value; }
    void send_P(int status, const char* type, const char* value) { send(status, type, value); }
    AsyncWebServerResponse* beginResponse(int status, const char*, String value) {
        return new AsyncWebServerResponse{status, value, {}};
    }
    void send(AsyncWebServerResponse* response) {
        code = response->code; body = response->body; headers = response->headers; delete response;
    }
};
class AsyncWebServer {
public:
    using Handler = std::function<void(AsyncWebServerRequest*)>;
    std::map<std::pair<std::string, int>, Handler> handlers;
    explicit AsyncWebServer(int) {}
    void on(const char* route, int method, Handler handler) { handlers[{route, method}] = handler; }
    template<class T> void addHandler(T*) {} void begin() {}
};
struct AsyncEventSource {
    explicit AsyncEventSource(const char*) {} void send(const char*, const char*, uint32_t) {}
};
struct BLEUUID { explicit BLEUUID(const char*) {} };
struct BLEAddress { String mac; std::string toString() { return mac; } };
struct BLEAdvertisedDevice {
    String mac = "c1:23:45:67:89:ab";
    bool matchingService = true;
    String name = "ZZK-SA2-89AB";
    bool haveServiceUUID() { return matchingService; }
    bool isAdvertisingService(BLEUUID) { return matchingService; }
    BLEAddress getAddress() { return {mac}; }
    bool haveName() { return !name.empty(); } std::string getName() { return name; }
    int getRSSI() { return -60; }
};
struct BLEScanResults {
    std::vector<BLEAdvertisedDevice> devices;
    int getCount() { return devices.size(); }
    BLEAdvertisedDevice getDevice(int i) { return devices.at(i); }
};
struct BLEScan {
    int starts = 0;
    BLEScanResults results;
    std::function<void()> onStart;
    BLEScanResults start(int, bool) { ++starts; if (onStart) onStart(); return results; }
    void clearResults() {} void stop() {} void setActiveScan(bool) {} void setInterval(int) {} void setWindow(int) {}
};
struct BLERemoteCharacteristic {
    int writes = 0; uint8_t lastValue = 0;
    bool canWrite() { return true; } bool canRead() { return true; } bool canNotify() { return true; }
    template<class T> void registerForNotify(T) {}
    uint8_t readUInt8() { return 75; }
    void writeValue(uint8_t* data, int, bool) { ++writes; lastValue = *data; }
};
static BLERemoteCharacteristic fakeCharacteristic;
struct BLERemoteService { BLERemoteCharacteristic* getCharacteristic(BLEUUID) { return &fakeCharacteristic; } };
static BLERemoteService fakeService;
class BLEClient;
struct BLEClientCallbacks {
    virtual ~BLEClientCallbacks() = default;
    virtual void onConnect(BLEClient*) {} virtual void onDisconnect(BLEClient*) {}
};
struct BLEClient {
    bool linked = false, autoDisconnect = true;
    int connects = 0, disconnects = 0, discoveries = 0;
    BLEClientCallbacks* callbacks = nullptr;
    std::function<void()> onConnecting;
    std::function<void()> onDiscovery;
    bool isConnected() { return linked; }
    void setClientCallbacks(BLEClientCallbacks* cb) { callbacks = cb; }
    bool connect(BLEAdvertisedDevice*) {
        ++connects; linked = true;
        if (callbacks) callbacks->onConnect(this);
        if (onConnecting) onConnecting();
        return true;
    }
    void finishDisconnect() { linked = false; if (callbacks) callbacks->onDisconnect(this); }
    void disconnect() { ++disconnects; if (autoDisconnect) finishDisconnect(); }
    void getServices() { ++discoveries; if (onDiscovery) onDiscovery(); }
    BLERemoteService* getService(BLEUUID) { return &fakeService; }
};
static BLEScan fakeScan;
static BLEClient fakeClient;
struct BLEDevice {
    static void init(const char*) {}
    static BLEScan* getScan() { return &fakeScan; }
    static BLEClient* createClient() { return &fakeClient; }
};
