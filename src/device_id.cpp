#include <Arduino.h>

#include "device_id.h"
#include "utils.h"

namespace fk {

DeviceId::DeviceId() {
    SerialNumber serialNumber;

    memcpy(values, serialNumber.toInts(), sizeof(uint32_t) * 4);
}

const char *DeviceId::toString() {
    sprintf(buffer, "%8lx-%8lx-%8lx-%8lx", values[0], values[1], values[2], values[3]);
    return buffer;
}

}
