#ifndef FK_DEVICE_ID_H_INCLUDED
#define FK_DEVICE_ID_H_INCLUDED

#include <cstdint>

namespace fk {

class DeviceId {
private:
    uint32_t values[4];
    char buffer[37];

public:
    DeviceId();

public:
    const char *toString();
    uint32_t *toInts() {
        return values;
    }

};

}

#endif
