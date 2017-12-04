#ifndef FK_UTILS_H_INCLUDED
#define FK_UTILS_H_INCLUDED

#include <cstdint>
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

}

#endif
