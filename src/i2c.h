#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <fk-module-protocol.h>

bool i2c_device_send(uint8_t address, const pb_field_t *fields, void *src);

bool i2c_device_send(uint8_t address, const void *ptr, size_t size);

bool i2c_device_receive(uint8_t address, const pb_field_t *fields, void *src);

#endif
