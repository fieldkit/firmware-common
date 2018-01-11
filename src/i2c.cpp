#include <Wire.h>
#include <cstdint>

#include "debug.h"
#include "i2c.h"

namespace fk {

bool TwoWireBus::begin() {
    bus->begin();
    return true;
}

bool TwoWireBus::begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest) {
    bus->begin(address);
    bus->onReceive(onReceive);
    bus->onRequest(onRequest);
    return true;
}

bool TwoWireBus::send(uint8_t address, const void *ptr, size_t size) {
    if (address > 0) {
        bus->beginTransmission(address);
        bus->write((uint8_t *)ptr, size);
        switch (bus->endTransmission()) {
        case 0:
            return true;
        default:
            return false;
        }
    } else {
        bus->write((uint8_t *)ptr, size);
    }

    return true;
}

size_t TwoWireBus::receive(uint8_t address, uint8_t *ptr, size_t size) {
    bus->requestFrom(address, FK_MODULE_PROTOCOL_MAX_MESSAGE);

    size_t bytes = 0;
    while (bus->available()) {
        ptr[bytes++] = bus->read();
        if (bytes == size) {
            break;
        }
    }

    return bytes;
}

size_t TwoWireBus::read(uint8_t *ptr, size_t size, size_t bytes) {
    fk_assert(bytes < size);
    for (size_t i = 0; i < bytes; ++i) {
        ptr[i] = bus->read();
    }
    return bytes;
}

}
