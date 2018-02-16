#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <Wire.h>

#include <fk-module-protocol.h>

#include "debug.h"

namespace fk {

constexpr uint8_t NumberOfTwoWireRetries = 3;

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

extern TwoWire Wire11and13;
extern TwoWire Wire4and3;

class Peripheral {
private:
    uint32_t acquiredAt{ 0 };

public:
    bool available() {
        return acquiredAt == 0;
    }

    bool tryAcquire() {
        if (available()) {
            acquire();
            return true;
        }
        return false;
    }

    // TODO: Specify who?
    void acquire() {
        fk_assert(available());
        acquiredAt = millis();
    }

    void release() {
        fk_assert(!available());
        acquiredAt = 0;
    }
};

class Peripherals {
private:
    Peripheral _twoWire1;
    Peripheral _twoWire2;
    Peripheral _twoWire3;

public:
    Peripheral &twoWire1() {
        return _twoWire1;
    }

    Peripheral &twoWire2() {
        return _twoWire2;
    }

    Peripheral &twoWire3() {
        return _twoWire3;
    }

};

extern Peripherals peripherals;

}

#endif
