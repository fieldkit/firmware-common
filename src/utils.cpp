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
    sprintf(buffer, "%8lx%8lx%8lx%8lx", values[0], values[1], values[2], values[3]);
    return buffer;
}

#if defined(PROGMEM)
#define FLASH_PROGMEM PROGMEM
#define FLASH_READ_DWORD(x) (pgm_read_dword_near(x))
#else
#define FLASH_PROGMEM
#define FLASH_READ_DWORD(x) (*(uint32_t*)(x))
#endif

static uint32_t crc_table[16] FLASH_PROGMEM = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t crc32_update(uint32_t crc, uint8_t data) {
    uint8_t tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = FLASH_READ_DWORD(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = FLASH_READ_DWORD(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    return crc;
}

uint32_t crc32_checksum(uint8_t *data, size_t size) {
    uint32_t crc = ~0;
    while (size-- > 0) {
        crc = crc32_update(crc, *(data++));
    }
    return ~crc;
}

}

namespace std {

void __throw_bad_alloc() {
    loginfof("Assert", "std::bad_alloc");
    while (true) {
    }
}

}
