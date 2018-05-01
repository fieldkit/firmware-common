#include "message_buffer.h"
#include "debug.h"

namespace fk {

bool MessageBuffer::write(ModuleQueryMessage &message) {
    return write(fk_module_WireMessageQuery_fields, message.forEncode());
}

bool MessageBuffer::write(ModuleReplyMessage &message) {
    return write(fk_module_WireMessageReply_fields, message.forEncode());
}

bool MessageBuffer::read(ModuleQueryMessage &message) {
    return read(fk_module_WireMessageQuery_fields, message.forDecode());
}

bool MessageBuffer::read(ModuleReplyMessage &message) {
    return read(fk_module_WireMessageReply_fields, message.forDecode());
}

bool MessageBuffer::read(AppQueryMessage &message) {
    return read(fk_app_WireMessageQuery_fields, message.forDecode());
}

bool MessageBuffer::write(AppReplyMessage &message) {
    return write(fk_app_WireMessageReply_fields, message.forEncode());
}

bool MessageBuffer::write(DataRecordMessage &message) {
    return write(fk_data_DataRecord_fields, message.forEncode());
}

bool MessageBuffer::write(const pb_field_t *fields, void *src) {
    size_t required = 0;

    if (!pb_get_encoded_size(&required, fields, src)) {
        return false;
    }

    auto stream = pb_ostream_from_buffer(ptr(), size());
    if (!pb_encode_delimited(&stream, fields, src)) {
        loginfof("Error", "Stream needs %d, we have %d", required, size());
        return false;
    }
    pos = stream.bytes_written;

    return true;
}

bool MessageBuffer::read(const pb_field_t *fields, void *src) {
    fk_assert(pos > 0);
    auto stream = pb_istream_from_buffer(ptr(), pos);
    if (!pb_decode_delimited(&stream, fields, src)) {
        return false;
    }
    return true;
}

bool TwoWireMessageBuffer::send(uint8_t address) {
    return bus->send(address, ptr(), pos);
}

bool TwoWireMessageBuffer::receive(uint8_t address) {
    pos = bus->receive(address, ptr(), size());
    return pos > 0;
}

bool TwoWireMessageBuffer::readIncoming(size_t bytes) {
    pos = bus->read(ptr(), size(), bytes);
    return pos > 0;
}

}
