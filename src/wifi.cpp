#include "wifi.h"
#include "utils.h"
#include "hardware.h"

namespace fk {

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
        log("N[%d] Failed (%s)", networkNumber, getWifiStatus());
        networkNumber++;
        return TaskEval::yield();
    }

    IpAddress4 ip{ WiFi.localIP() };
    log("N[%d] Connected (%s) (%s)", networkNumber, getWifiStatus(), ip.toString());

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
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

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
    else {
        push(delay);
    }
}

bool Wifi::readyToServe() {
    return WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_CONNECTED;
}

bool Wifi::isListening() {
    return WiFi.status() == WL_AP_LISTENING;
}

bool Wifi::isDisconnected() {
    return WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS;
}

void Wifi::ensureDisconnected() {
    if (!isDisconnected()) {
        WiFi.disconnect();
        while (!isDisconnected()) {
            ::delay(1000);
            log("Disconnecting... (%s)", getWifiStatus());
        }
        log("Disconnected (%s)", getWifiStatus());
    }
}

void Wifi::idle() {
    if (WiFi.status() == WL_NO_SHIELD) {
        return;
    }

    uint8_t newStatus = WiFi.status();
    if (newStatus != status) {
        log("Changed: %s", getWifiStatus());
        status = newStatus;
    }

    auto settings = state->getNetworkSettings();
    if (version == settings.version) {
        if (readyToServe()) {
            service(listen);
        }
        return;
    }

    log("New configuration...");
    version = settings.version;
    listen.end();
    ensureDisconnected();
    cancel();

    push(connectToWifiAp);
}

}
