#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <fk-module-protocol.h>

bool i2c_device_send(uint8_t address, const void *ptr, size_t size);

size_t i2c_device_receive(uint8_t address, uint8_t *ptr, size_t size);

size_t i2c_device_read(uint8_t *ptr, size_t size, size_t bytes);

#endif
