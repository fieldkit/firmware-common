#include <Wire.h>

#include "module_messages.h"
#include "i2c.h"

namespace fk {

bool QueryMessage::send(uint8_t address) {
    return i2c_device_send(address, fk_module_WireMessageQuery_fields, &message);
}

size_t QueryMessage::write(uint8_t *buffer, size_t size) {
    auto stream = pb_ostream_from_buffer(buffer, size);
    if (!pb_encode_delimited(&stream, fields, &message)) {
        return 0;
    }
    return stream.bytes_written;
}

bool ReplyMessage::receive(uint8_t address) {
    message.error.message.funcs.decode = pb_decode_string;
    message.error.message.arg = pool;
    message.capabilities.name.funcs.decode = pb_decode_string;
    message.capabilities.name.arg = pool;
    message.sensorCapabilities.name.funcs.decode = pb_decode_string;
    message.sensorCapabilities.name.arg = pool;
    message.sensorCapabilities.unitOfMeasure.funcs.decode = pb_decode_string;
    message.sensorCapabilities.unitOfMeasure.arg = pool;

    return i2c_device_receive(address, fk_module_WireMessageReply_fields, &message);
}

size_t ReplyMessage::write(uint8_t *buffer, size_t size) {
    message.error.message.funcs.encode = pb_encode_string;
    message.capabilities.name.funcs.encode = pb_encode_string;
    message.sensorCapabilities.name.funcs.encode = pb_encode_string;
    message.sensorCapabilities.unitOfMeasure.funcs.encode = pb_encode_string;

    auto stream = pb_ostream_from_buffer(buffer, size);
    if (!pb_encode_delimited(&stream, fields, &message)) {
        return 0;
    }
    return stream.bytes_written;
}

bool MessageBuffer::write(QueryMessage &message) {
    return write(fk_module_WireMessageQuery_fields, &message.m());
}

bool MessageBuffer::write(ReplyMessage &message) {
    return write(fk_module_WireMessageReply_fields, &message.m());
}

bool MessageBuffer::read(QueryMessage &message) {
    return read(fk_module_WireMessageQuery_fields, &message.m());
}

bool MessageBuffer::read(ReplyMessage &message) {
    return read(fk_module_WireMessageReply_fields, &message.m());
}

bool MessageBuffer::write(const pb_field_t *fields, void *src) {
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fields, src)) {
        return false;
    }
    length = stream.bytes_written;
    return true;
}

bool MessageBuffer::read(const pb_field_t *fields, void *src) {
    auto stream = pb_istream_from_buffer(buffer, length);
    if (!pb_decode_delimited(&stream, fields, src)) {
        return false;
    }
    return true;
}

}
