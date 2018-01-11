#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <Wire.h>

#include <fk-module-protocol.h>

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
    bool begin();
    bool begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest);

    bool send(uint8_t address, const void *ptr, size_t size);
    size_t receive(uint8_t address, uint8_t *ptr, size_t size);
    size_t read(uint8_t *ptr, size_t size, size_t bytes);

};

}

#endif
