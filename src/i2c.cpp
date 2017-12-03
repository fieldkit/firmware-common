#include <Wire.h>
#include <cstdint>

#include "debug.h"
#include "i2c.h"

namespace fk {

bool i2c_begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest) {
    Wire.begin(address);
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
    return true;
}

bool i2c_begin(void) {
    Wire.begin();
    return true;
}

bool i2c_device_send(uint8_t address, const void *ptr, size_t size) {
    if (address > 0) {
        Wire.beginTransmission(address);
        Wire.write((uint8_t *)ptr, size);
        switch (Wire.endTransmission()) {
        case 0:
            return true;
        default:
            return false;
        }
    } else {
        Wire.write((uint8_t *)ptr, size);
    }

    return true;
}

size_t i2c_device_receive(uint8_t address, uint8_t *ptr, size_t size) {
    Wire.requestFrom(address, FK_MODULE_PROTOCOL_MAX_MESSAGE);

    size_t bytes = 0;
    while (Wire.available()) {
        ptr[bytes++] = Wire.read();
        if (bytes == size) {
            break;
        }
    }

    return bytes;
}

size_t i2c_device_read(uint8_t *ptr, size_t size, size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
        ptr[i] = Wire.read();
    }
    return bytes;
}

}
