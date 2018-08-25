#ifndef FK_WIFI_TOOLS_H_INCLUDED
#define FK_WIFI_TOOLS_H_INCLUDED

#include <cinttypes>
#include <cstdio>

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
    uint32_t cached_{ 0 };

public:
    bool cached(const char *hostname);

    uint32_t ip() {
        return cached_;
    }

};

const char *getWifiStatus(uint8_t status);

const char *getWifiStatus();

}

#endif
