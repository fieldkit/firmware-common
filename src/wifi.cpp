#include "wifi.h"

namespace fk {

static const char *getWifiStatus(uint8_t status) {
    switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    case WL_AP_LISTENING: return "WL_AP_LISTENING";
    case WL_AP_CONNECTED: return "WL_AP_CONNECTED";
    case WL_AP_FAILED: return "WL_AP_FAILED";
    case WL_PROVISIONING: return "WL_PROVISIONING";
    case WL_PROVISIONING_FAILED: return "WL_PROVISIONING_FAILED";
    default: return "Unknown";
    }
}

static const char *getWifiStatus() {
    return getWifiStatus(WiFi.status());
}

TaskEval ConnectToWifiAp::task() {
    if (networkNumber >= MaximumRememberedNetworks) {
        log("No more networks (%s)", getWifiStatus());
        return TaskEval::error();
    }

    auto settings = state->getNetworkSettings();
    auto network = settings.networks[networkNumber];

    if (network.ssid[0] == 0) {
        log("N[%d] No network configured (%s)", networkNumber, getWifiStatus());
        networkNumber++;
        return TaskEval::yield();
    }

    log("N[%d] Connecting to AP '%s'... (%s)", networkNumber, network.ssid, getWifiStatus());
    if (WiFi.begin(network.ssid, network.password) != WL_CONNECTED) {
        networkNumber++;
        return TaskEval::yield();
    }

    log("N[%d] Connected (%s)", networkNumber, getWifiStatus());

    return TaskEval::done();
}

TaskEval CreateWifiAp::task() {
    auto name = "FK-HELP";

    log("Creating AP '%s'... (%s)", name, getWifiStatus());
    auto status = WiFi.beginAP(name);
    if (status != WL_AP_LISTENING) {
        return TaskEval::error();
    }

    return TaskEval::done();
}

Wifi::Wifi(CoreState &state, AppServicer &servicer)
    : ActiveObject("Wifi"), state(&state), connectToWifiAp(state), createWifiAp(state), listen(ServerPort, servicer) {
}

void Wifi::begin() {
    WiFi.setPins(8, 7, 4, 2);

    if (WiFi.status() == WL_NO_SHIELD) {
        log("Error: no wifi");
        return;
    }
}

void Wifi::done(Task &task) {
}

void Wifi::error(Task &task) {
    if (areSame(task, connectToWifiAp)) {
        push(createWifiAp);
    }
}

void Wifi::idle() {
    if (WiFi.status() == WL_NO_SHIELD) {
        return;
    }

    auto settings = state->getNetworkSettings();
    if (version == settings.version) {
        service(listen);
        return;
    }

    log("New configuration...");
    version = settings.version;
    listen.end();
    cancel();

    if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_LISTENING) {
        WiFi.disconnect();
        while (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_LISTENING) {

        }
        log("Disconnected (%s)", getWifiStatus());
    }

    push(connectToWifiAp);
}

}
