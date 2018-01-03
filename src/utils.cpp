#include <WiFi101.h>

#include "utils.h"
#include "debug.h"

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

void HttpResponseParser::begin() {
    buffer[0] = pos = spacesSeen = 0;
}

void HttpResponseParser::write(uint8_t c) {
    if (spacesSeen < 2) {
        if (c == ' ') {
            spacesSeen++;
            if (spacesSeen == 2) {
                statusCode = atoi(buffer);
            }
            buffer[0] = pos = 0;
        } else {
            if (pos < MaxStatusCodeLength - 1) {
                buffer[pos++] = c;
                buffer[pos] = 0;
            }
        }
    }
}

SerialNumber::SerialNumber() {
    volatile uint32_t *ptr1 = (volatile uint32_t *)0x0080A00C;
    values[0] = *ptr1;
    volatile uint32_t *ptr = (volatile uint32_t *)0x0080A040;
    values[1] = *ptr;
    ptr++;
    values[2] = *ptr;
    ptr++;
    values[3] = *ptr;
}

const char *SerialNumber::toString() {
    sprintf(buffer, "%8lx-%8lx-%8lx-%8lx", values[0], values[1], values[2], values[3]);
    return buffer;
}

}

namespace std {

void __throw_bad_alloc() {
    debugfln("std::bad_alloc");
    while (true) {
    }
}

}
