#ifndef FK_DEVICE_ID_H_INCLUDED
#define FK_DEVICE_ID_H_INCLUDED

#include <cstdint>

namespace fk {

class DeviceId {
private:
    size_t len{ 0 };
    uint8_t data[4 * 4];
    char buffer[37];

public:
    DeviceId();

public:
    const char *toString();
    size_t length() {
        return len;
    }
    uint8_t *toBuffer() {
        return data;
    }

};

}

#endif
