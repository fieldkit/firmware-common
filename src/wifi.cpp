#include <SPI.h>
#ifdef FK_CORE
#include <Base64.h>
#endif

#include "wifi.h"
#include "utils.h"
#include "hardware.h"
#include "static_wifi_allocator.h"

namespace fk {

StaticWiFiAllocator staticWiFiAllocator;

constexpr uint32_t WifiAwakenInterval = 1000 * 60 * 1;
constexpr uint32_t ScanDuration = 10 * 1000;

#ifdef FK_CORE
void getAccessPointName(char *name, size_t size) {
    auto length = base64_enc_len(deviceId.length());
    char unique[length + 3];
    unique[0] = 'F';
    unique[1] = 'K';
    unique[2] = '-';
    base64_encode(unique + 3, (char *)deviceId.toBuffer(), deviceId.length());
    unique[length + 3 - 1] = 0; // Trim '='
    strncpy(name, unique, size);
}
#else
void getAccessPointName(char *name, size_t size) {
    strncpy(name, "FK-UNKONWN", size);
}
#endif

TaskEval ConnectToWifiAp::task() {
    if (networkNumber >= MaximumRememberedNetworks) {
        networkNumber = 0;
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
    char name[32];
    getAccessPointName(name, sizeof(name));

    log("Creating AP '%s'... (%s)", name, getWifiStatus());
    IPAddress ip{ 192, 168, 2, 1 };
    auto status = WiFi.beginAP(name, 1, ip);
    if (status != WL_AP_LISTENING) {
        return TaskEval::error();
    }

    return TaskEval::done();
}

static const char *encryptionType(int32_t type) {
    switch (type) {
    case ENC_TYPE_WEP: return "WEP";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_NONE: return "None";
    case ENC_TYPE_AUTO: return "Auto";
    }
    return "Unknown";
}

TaskEval ScanNetworks::task() {
    if (begunAt == 0) {
        log("Starting scan (%s)", getWifiStatus());

        if (m2m_wifi_request_scan(M2M_WIFI_CH_ALL) < 0) {
            return TaskEval::error();
        }

        begunAt = millis() + ScanDuration;
    }

    if (begunAt > millis()) {
        return TaskEval::idle();
    }

    if ((WiFi.status() & WL_SCAN_COMPLETED) == WL_SCAN_COMPLETED) {
        auto numberOfNetworks = m2m_wifi_get_num_ap_found();

        log("Scan done: found %d, took %ldms (%s)", numberOfNetworks, millis() - begunAt, getWifiStatus());

        for (auto i = 0; i < numberOfNetworks; i++) {
            log("Network[%d] %s %lddbm %s", i, WiFi.SSID(i), WiFi.RSSI(i), encryptionType(WiFi.encryptionType(i)));
        }

        begunAt = 0;

        return TaskEval::done();
    }

    return TaskEval::idle();
}

Wifi::Wifi(CoreState &state, AppServicer &servicer)
    : ActiveObject("Wifi"), state(&state), connectToWifiAp(state), createWifiAp(state), listen(ServerPort, servicer) {
}

void Wifi::begin() {
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);

    WiFiSocketClass::allocator = &staticWiFiAllocator;

    if (WiFi.status() == WL_NO_SHIELD) {
        log("Error: no wifi (%d, %d, %d, %d)", Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST, Hardware::WIFI_PIN_EN);
        return;
    }
}

void Wifi::done(Task &task) {
    if (areSame(task, scanNetworks)) {
        push(createWifiAp);
    }
    lastActivityAt = millis();
}

void Wifi::error(Task &task) {
    if (areSame(task, connectToWifiAp)) {
        #ifdef FK_NATURALIST
        disable();
        #else
        push(createWifiAp);
        #endif
    }
    else if (areSame(task, createWifiAp)) {
        disable();
    }
    else {
        push(delay);
    }
    lastActivityAt = millis();
}

bool Wifi::readyToServe() {
    return status == WL_CONNECTED || status == WL_AP_CONNECTED;
}

bool Wifi::isListening() {
    return status == WL_AP_LISTENING;
}

void Wifi::ensureDisconnected() {
    if (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS) {
        return;
    }

    WiFi.disconnect();

    while (!(WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_IDLE_STATUS)) {
        ::delay(1000);
        log("Disconnecting(%s)...", getWifiStatus());
    }

    log("Disconnected(%s)", getWifiStatus());
}

void Wifi::disable() {
    listen.end();
    WiFi.end();
    lastActivityAt = millis();
    disabled = true;
    state->updateIp(0);

    // Allow me to explain:
    // I was seeing this very strange problem where after a Disable
    // the WDT would kick off. It was always preceeded by fkfs
    // activity and so things eventually led me to the SPI bus code.
    // I was inside of sd_raw, in the code for sd_raw_command.
    // There's a call to sd_raw_flush and that's where the hang
    // occured. In there it waits until the "Data Register is Empty"
    // (ATSAMD SerCOM.CPP line 305) I'm assuming there's some kind
    // of flush that doesn't happen. This "reset" fixes the problem.
    // If I do this, and don't do the call to maxLowPowerMode
    // anymore and also move the log call you see below to after the
    // reset of the bus things work more often. There is still a WDT
    // reset occuring, though just doesn't seem to happen around here.
    SPI.end();
    SPI.begin();

    log("Disabled");
}

void Wifi::idle() {
    if (disabled) {
        if (millis() - lastActivityAt > WifiAwakenInterval) {
            log("Enabling...");
            begin();
            lastActivityAt = millis();
            disabled = false;
            version = 0;
        }
        return;
    }

    auto newStatus = WiFi.status();
    if (newStatus != status) {
        log("Changed: %s", getWifiStatus());
        status = newStatus;
        lastActivityAt = millis();
        state->updateIp(WiFi.localIP());
    }

    if (status == WL_NO_SHIELD) {
        return;
    }

    auto settings = state->getNetworkSettings();
    if (version == settings.version) {
        if (readyToServe()) {
            service(listen);
        }
        else {
            listen.end();
        }

        state->setBusy(listen.hasConnection());
        if (!busy && listen.inactive()) {
            if (millis() - lastActivityAt > InactivityTimeout) {
                if (isListening() || readyToServe())  {
                    disable();
                }
            }
        }
        return;
    }

    log("New configuration...");
    version = settings.version;
    ensureDisconnected();
    listen.end();
    cancel();

    push(connectToWifiAp);
}

}
