#ifndef FK_TWO_WIRE_H_INCLUDED
#define FK_TWO_WIRE_H_INCLUDED

#include <Wire.h>

#include <fk-module-protocol.h>

#include "debug.h"
#include "peripherals.h"

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
    TwoWire *twoWire() {
        return bus;
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

public:
    static constexpr uint8_t I2C_PIN_SDA1 = 20; // PIN_WIRE_SDA
    static constexpr uint8_t I2C_PIN_SCL1 = 21; // PIN_WIRE_SCL

    static constexpr uint8_t I2C_PIN_SDA2 = 4;
    static constexpr uint8_t I2C_PIN_SCL2 = 3;

    static constexpr uint8_t I2C_PIN_SDA3 = 11;
    static constexpr uint8_t I2C_PIN_SCL3 = 13;

};

extern TwoWire Wire4and3;

}

#endif
