#ifndef FK_UTILS_H_INCLUDED
#define FK_UTILS_H_INCLUDED

#include <IPAddress.h>
#include <WiFi101.h>

#include <cstdint>
#include <cstdio>

#include "url_parser.h"
#include "http_response_writer.h"
#include "http_response_parser.h"
#include "print_helpers.h"

namespace fk {

class IpAddress4 {
private:
    char str[3 * 4 + 4] = { 0 };
    union {
        uint8_t bytes[4];
        uint32_t dword;
    } a;

public:
    IpAddress4() {
    }

    IpAddress4(uint32_t ip) {
        a.dword = ip;
    }

public:
    const char *toString() {
        snprintf(str, sizeof(str), "%d.%d.%d.%d", a.bytes[0], a.bytes[1], a.bytes[2], a.bytes[3]);
        return str;
    }
};

class CachedDnsResolution {
private:
    ::IPAddress cachedAddress;

public:
    bool cached(const char *hostname) {
        if ((uint32_t)cachedAddress == (uint32_t)0) {
            if (!WiFi.hostByName(hostname, cachedAddress)) {
                return false;
            }
        }
        return true;
    }

    uint32_t ip() {
        return cachedAddress;
    }

};

const char *getWifiStatus(uint8_t status);

const char *getWifiStatus();

class SerialNumber {
    static constexpr size_t MaximumSerialNumberLengthDwords = 4;
    static constexpr size_t MaximumSerialNumberLengthBytes = MaximumSerialNumberLengthDwords * 4;

private:
    uint32_t values[MaximumSerialNumberLengthDwords];
    char buffer[37];

public:
    SerialNumber();

public:
    const char *toString();
    uint32_t *toInts() {
        return values;
    }

};

uint32_t crc32_update(uint32_t crc, uint8_t data);

uint32_t crc32_checksum(uint8_t *data, size_t size);

}

#endif
