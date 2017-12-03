#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <fk-module-protocol.h>

namespace fk {

using WireOnReceiveHandler = void (*)(int);

using WireOnRequestHandler = void (*)(void);

bool i2c_begin(uint8_t address, WireOnReceiveHandler onReceive, WireOnRequestHandler onRequest);

bool i2c_begin(void);

bool i2c_device_send(uint8_t address, const void *ptr, size_t size);

size_t i2c_device_receive(uint8_t address, uint8_t *ptr, size_t size);

size_t i2c_device_read(uint8_t *ptr, size_t size, size_t bytes);

}

#endif
