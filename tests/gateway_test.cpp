#include <cassert>
#include <iostream>
#include <future>
#include "../src/main.cpp"

AsyncWebServerRequest call(const char* route, int method, const char* key = "", const char* value = "") {
    AsyncWebServerRequest request;
    (method == HTTP_POST ? request.form : request.query)[key].text = value;
    server.handlers.at({route, method})(&request);
    return request;
}

int main() {
    setupWebServer();
    assert(call("/maintenance", HTTP_POST, "action", "release").code == 409);
    assert(call("/maintenance", HTTP_POST, "action", "invalid").code == 400);
    assert(call("/control", HTTP_GET, "action", "1").code == 503);
    assert(call("/control", HTTP_GET, "action", "2").code == 400);
    assert(call("/status", HTTP_GET).headers.at("Cache-Control") == "no-store");
    assert(call("/bind", HTTP_GET, "mac", "invalid").code == 400);
    assert(call("/bind", HTTP_GET, "mac", "C1:23:45:67:89:AB").code == 200);
    const String boundMac = state.targetMacAddress;
    const auto savedBinding = preferences.values;
    // Neither unbound devices nor a DFU bootloader with no switch service match.
    BLEAdvertisedDevice other; other.mac = "c1:00:00:00:00:00";
    BLEAdvertisedDevice dfu; dfu.matchingService = false; dfu.name = "XIAO_DFU";
    fakeScan.results.devices = {other, dfu};
    processBle();
    assert(fakeClient.connects == 0);
    fakeScan.results.devices = {BLEAdvertisedDevice()};
    processBle();
    assert(state.connected && fakeClient.discoveries == 1);
    assert(call("/control", HTTP_GET, "action", "1").code == 200);
    processBle();
    assert(fakeCharacteristic.writes == 1 && fakeCharacteristic.lastValue == 1);

    // Discard a queued light command; don't claim release until disconnect ends.
    call("/control", HTTP_GET, "action", "0");
    fakeClient.autoDisconnect = false;
    assert(call("/maintenance", HTTP_POST, "action", "release").code == 202);
    assert(call("/control", HTTP_GET, "action", "0").code == 409);
    assert(call("/scan", HTTP_GET).code == 409);
    assert(call("/bind", HTTP_GET, "mac", "c2:00:00:00:00:00").code == 409);
    processBle();
    assert(state.maintenance.phase() == OtaMaintenance::Phase::Releasing);
    fakeNow = 30000; // Slow disconnect does not shorten the maintenance window.
    fakeClient.finishDisconnect();
    processBle();
    assert(state.maintenance.remainingSeconds(fakeNow) == 180);
    assert(state.battery == -1 && !state.connected);
    const int scans = fakeScan.starts;
    for (int i = 0; i < 100; ++i) processBle();
    assert(fakeScan.starts == scans && fakeCharacteristic.writes == 1);
    assert(state.targetMacAddress == boundMac && preferences.values == savedBinding);
    fakeClient.autoDisconnect = true;
    fakeNow += 180000;
    processBle();
    assert(state.connected && fakeClient.discoveries == 2);
    assert(fakeCharacteristic.writes == 1); // No replay after reconnect.

    // An HTTP release arriving inside the scan must suppress the queued target.
    disconnectCurrentDevice();
    const int connects = fakeClient.connects;
    fakeScan.onStart = [] { call("/maintenance", HTTP_POST, "action", "release"); };
    processBle();
    assert(fakeClient.connects == connects);
    fakeScan.onStart = nullptr;
    processBle();
    assert(state.maintenance.phase() == OtaMaintenance::Phase::Active);

    // A release arriving during connect/GATT discovery must tear the link down.
    call("/maintenance", HTTP_POST, "action", "resume");
    fakeClient.onConnecting = [] { call("/maintenance", HTTP_POST, "action", "release"); };
    processBle();
    assert(!fakeClient.linked && !state.connected);
    fakeClient.onConnecting = nullptr;
    processBle();
    call("/maintenance", HTTP_POST, "action", "resume");
    fakeClient.onDiscovery = [] { call("/maintenance", HTTP_POST, "action", "release"); };
    processBle();
    assert(!fakeClient.linked && !state.connected);
    fakeClient.onDiscovery = nullptr;
    processBle();
    call("/maintenance", HTTP_POST, "action", "resume");
    processBle();
    assert(state.connected && !state.maintenance.blocksBle());
    assert(preferences.values == savedBinding);

    // Early resume must wait for an outstanding disconnect before reconnecting.
    fakeClient.autoDisconnect = false;
    call("/maintenance", HTTP_POST, "action", "release");
    processBle();
    const int beforeEarlyResume = fakeClient.connects;
    call("/maintenance", HTTP_POST, "action", "resume");
    processBle();
    assert(fakeClient.connects == beforeEarlyResume && !state.connected);
    fakeClient.finishDisconnect();
    fakeClient.autoDisconnect = true;
    processBle();
    assert(state.connected && fakeClient.connects == beforeEarlyResume + 1);

    // Shared BLE operations cannot race a web scan/bind with the loop.
    bleMutex.lock();
    std::async(std::launch::async, [] {
        assert(call("/scan", HTTP_GET).code == 409);
        assert(call("/bind", HTTP_GET, "mac", "c2:00:00:00:00:00").code == 409);
    }).get();
    bleMutex.unlock();
    std::cout << "Gateway HTTP gates, scan/connect races, async disconnect, reconnect and binding passed\n";
}
