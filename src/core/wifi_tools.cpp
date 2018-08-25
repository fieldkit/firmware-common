#include <WiFi101.h>
#include <IPAddress.h>

#include "wifi_tools.h"

namespace fk {

const char *getWifiStatus(uint8_t status) {
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

const char *getWifiStatus() {
    return getWifiStatus(WiFi.status());
}

bool CachedDnsResolution::cached(const char *hostname) {
    if (cached_ == (uint32_t)0) {
        IPAddress address;
        if (!WiFi.hostByName(hostname, address)) {
            return false;
        }
        cached_ = (uint32_t)address;
    }
    return true;
}

}
