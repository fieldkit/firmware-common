#ifndef FK_DEVICE_ID_H_INCLUDED
#define FK_DEVICE_ID_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include <cstdint>

#include "two_wire.h"

namespace fk {

class DeviceId {
private:
    size_t len{ 0 };
    uint8_t data[4 * 4];
    char buffer[37];

public:
    bool initialize(TwoWireBus &bus);

public:
    const char *toString();

    size_t length() {
        return len;
    }

    uint8_t *toBuffer() {
        return data;
    }

    lws::BufferPtr toBufferPtr() {
        return lws::BufferPtr{ data, len };
    }

private:
};

extern DeviceId deviceId;

}

#endif
