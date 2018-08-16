#include <Arduino.h>
#include <Wire.h>

#include "debug.h"
#include "device_id.h"
#include "serial_number.h"

namespace fk {

class MacAddressEeprom {
private:
    TwoWireBus *bus;
    uint8_t address{ 0x50 };

public:
    MacAddressEeprom(TwoWireBus &bus) : bus(&bus) {
    }

public:
    bool read128bMac(uint8_t *id) {
        uint8_t buffer[] = { 0xf8 };
        if (!bus->send(address, buffer, sizeof(buffer))) {
            return false;
        }

        bus->receive(address, id, 8);

        return true;
    }
};

bool DeviceId::initialize(TwoWireBus &bus) {
    MacAddressEeprom macAddressChip{ bus };
    for (auto i = 0; i < 3; ++i) {
        memset(data, 0, sizeof(data));
        if (!macAddressChip.read128bMac(data)) {
            SerialNumber serialNumber;
            memcpy(data, (uint8_t *)serialNumber.toInts(), sizeof(uint32_t) * 4);
            len = 16;
        } else {
            len = 8;
        }

        for (size_t j = 0; j < len; ++j) {
            if (data[j] != 0xff || data[j] != 0x0) {
                return true;
            }
        }

        delay(10);
    }
    return false;
}

const char *DeviceId::toString() {
    for (size_t i = 0; i < len; ++i) {
        sprintf(buffer + (i * 2), "%02x", data[i]);
    }
    return buffer;
}

DeviceId deviceId;

}
