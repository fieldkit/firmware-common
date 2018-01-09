#include <Arduino.h>
#include <Wire.h>

#include "debug.h"
#include "device_id.h"
#include "utils.h"

namespace fk {

class MacAddressEeprom {
private:
    uint8_t address{ 0x50 };

public:
    bool read128bMac(uint8_t *id) {
        Wire.begin();

        Wire.beginTransmission(address);
        Wire.write(0xf8);
        if (Wire.endTransmission() != 0) {
            return false;
        }

        Wire.requestFrom(address, 8);

        auto valid = false;
        uint8_t index = 0;
        while (Wire.available() && index < 8) {
            uint8_t value = Wire.read();
            if (value != 0) {
                valid = true;
            }
            id[index++] = value;
        }

        return valid;
    }
};

DeviceId::DeviceId() {
    MacAddressEeprom macAddressChip;
    if (!macAddressChip.read128bMac(data)) {
        SerialNumber serialNumber;
        memcpy(data, (uint8_t *)serialNumber.toInts(), sizeof(uint32_t) * 4);
        len = 16;
    } else {
        len = 8;
    }
}

const char *DeviceId::toString() {
    for (size_t i = 0; i < len; ++i) {
        sprintf(buffer + (i * 2), "%02x", data[i]);
    }
    return buffer;
}

}
