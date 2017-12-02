#ifndef FK_MODULE_MESSAGES_H_INCLUDED
#define FK_MODULE_MESSAGES_H_INCLUDED

#include <fk-module-protocol.h>

#include "pool.h"
#include "protobuf.h"

namespace fk {

// TODO: Make this a template?

class MessageWrapper {
protected:
    const pb_field_t *fields;

public:
    MessageWrapper(const pb_field_t *fields) : fields(fields) {
    }

};

class QueryMessage : public MessageWrapper {
private:
    fk_module_WireMessageQuery message = fk_module_WireMessageQuery_init_default;
    Pool *pool;

public:
    QueryMessage(Pool *pool) : MessageWrapper(fk_module_WireMessageQuery_fields), pool(pool) {
    }

    fk_module_WireMessageQuery *forDecode() {
        return &message;
    }

    fk_module_WireMessageQuery *forEncode() {
        return &message;
    }

    fk_module_WireMessageQuery &m() {
        return message;
    }

};

class ReplyMessage : public MessageWrapper {
private:
    fk_module_WireMessageReply message = fk_module_WireMessageReply_init_default;
    Pool *pool;

public:
    ReplyMessage(Pool *pool) : MessageWrapper(fk_module_WireMessageReply_fields), pool(pool) {
    }

    fk_module_WireMessageReply *forDecode() {
        message.error.message.funcs.decode = pb_decode_string;
        message.error.message.arg = pool;
        message.capabilities.name.funcs.decode = pb_decode_string;
        message.capabilities.name.arg = pool;
        message.sensorCapabilities.name.funcs.decode = pb_decode_string;
        message.sensorCapabilities.name.arg = pool;
        message.sensorCapabilities.unitOfMeasure.funcs.decode = pb_decode_string;
        message.sensorCapabilities.unitOfMeasure.arg = pool;
        return &message;
    }

    fk_module_WireMessageReply *forEncode() {
        message.error.message.funcs.encode = pb_encode_string;
        message.capabilities.name.funcs.encode = pb_encode_string;
        message.sensorCapabilities.name.funcs.encode = pb_encode_string;
        message.sensorCapabilities.unitOfMeasure.funcs.encode = pb_encode_string;
        return &message;
    }

    fk_module_WireMessageReply &m() {
        return message;
    }

};

class MessageBuffer {
private:
    uint8_t buffer[FK_MODULE_PROTOCOL_MAX_MESSAGE];
    size_t length { 0 };

public:
    const uint8_t *ptr() {
        return buffer;
    }

    size_t size() {
        return length;
    }

    void clear() {
        length = 0;
    }

    bool empty() {
        return length == 0;
    }

    void append(uint8_t c) {
        buffer[length++] = c;
    }

    bool write(QueryMessage &message);

    bool write(ReplyMessage &message);

    bool read(QueryMessage &message);

    bool read(ReplyMessage &message);

    bool send(uint8_t address);

    bool receive(uint8_t address);

private:
    bool write(const pb_field_t *fields, void *src);
    bool read(const pb_field_t *fields, void *src);

};

}

#endif
