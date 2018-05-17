#ifdef FK_CORE
#include <Base64.h>
#endif

#include "wifi_connections.h"
#include "utils.h"

namespace fk {

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
        return TaskEval::busy();
    }

    log("N[%d] Connecting to AP '%s'... (%s)", networkNumber, network.ssid, getWifiStatus());
    if (WiFi.begin(network.ssid, network.password) != WL_CONNECTED) {
        log("N[%d] Failed (%s)", networkNumber, getWifiStatus());
        networkNumber++;
        return TaskEval::busy();
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

        begunAt = millis() + WifiScanDuration;
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

}
