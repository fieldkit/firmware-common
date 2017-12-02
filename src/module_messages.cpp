#include <Wire.h>

#include "module_messages.h"
#include "debug.h"
#include "i2c.h"

namespace fk {

bool MessageBuffer::send(uint8_t address) {
    return i2c_device_send(address, buffer, length);
}

bool MessageBuffer::receive(uint8_t address) {
    length = i2c_device_receive(address, buffer, sizeof(buffer));
    return length > 0;
}

bool MessageBuffer::write(QueryMessage &message) {
    return write(fk_module_WireMessageQuery_fields, message.forEncode());
}

bool MessageBuffer::write(ReplyMessage &message) {
    return write(fk_module_WireMessageReply_fields, message.forEncode());
}

bool MessageBuffer::read(QueryMessage &message) {
    return read(fk_module_WireMessageQuery_fields, message.forDecode());
}

bool MessageBuffer::read(ReplyMessage &message) {
    return read(fk_module_WireMessageReply_fields, message.forDecode());
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
