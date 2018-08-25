#ifndef FK_TWO_WIRE_H_INCLUDED
#define FK_TWO_WIRE_H_INCLUDED

#include <Wire.h>

#include <fk-module-protocol.h>

#include "debug.h"
#include "peripherals.h"
#include "hardware.h"

namespace fk {

using WireOnReceiveHandler = void (*)(int);

using WireOnRequestHandler = void (*)(void);

class TwoWireBus {
private:
    TwoWire *bus;

public:
    TwoWireBus(TwoWire &bus) : bus(&bus) {
    }

public:
    bool begin(uint32_t speed = 0);
    bool begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest);
    bool send(uint8_t address, const char *ptr);
    bool send(uint8_t address, const void *ptr, size_t size);
    size_t receive(uint8_t address, uint8_t *ptr, size_t size);
    size_t read(uint8_t *ptr, size_t size, size_t bytes);
    void flush();
    void end();

public:
    uint8_t requestFrom(uint8_t address, size_t quantity, bool stopBit);
    uint32_t available();
    uint8_t read();
    uint8_t endTransmission();

};

#ifdef FK_HARDWARE_WIRE11AND13_ENABLE
extern TwoWire Wire11and13;
#endif
extern TwoWire Wire4and3;

}

#endif
