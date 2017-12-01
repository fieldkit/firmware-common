#include <cstdint>
#include <Wire.h>

#include "i2c.h"
#include "debug.h"

bool i2c_device_send(uint8_t address, const pb_field_t *fields, void *src) {
    uint8_t buffer[FK_MODULE_PROTOCOL_MAX_MESSAGE];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fields, src)) {
        return false;
    }

    return i2c_device_send(address, buffer, stream.bytes_written);
}

bool i2c_device_send(uint8_t address, const void *ptr, size_t size) {
    if (address > 0) {
        Wire.beginTransmission(address);
        Wire.write((uint8_t *)ptr, size);
        switch (Wire.endTransmission()) {
        case 0: return true;
        default: return false;
        }
    }
    else {
        Wire.write((uint8_t *)ptr, size);
    }

    return true;
}

bool i2c_device_receive(uint8_t address, const pb_field_t *fields, void *src) {
    uint8_t buffer[FK_MODULE_PROTOCOL_MAX_MESSAGE];
    Wire.requestFrom(address, FK_MODULE_PROTOCOL_MAX_MESSAGE);

    auto bytes = 0;
    while (Wire.available()) {
        buffer[bytes++] = Wire.read();
        if (bytes == FK_MODULE_PROTOCOL_MAX_MESSAGE) {
            // We don't know how big the actual message is... so we're actually
            // filling this buffer and learning during the decode how things
            // went. This could be improved.
            break;
        }
    }

    auto stream = pb_istream_from_buffer(buffer, bytes);
    if (!pb_decode_delimited(&stream, fields, src)) {
        debugfln("fk: bad message (%d)", bytes);
        return false;
    }

    return true;
}
