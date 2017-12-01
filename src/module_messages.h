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

    fk_module_WireMessageQuery &m() {
        return message;
    }

    operator fk_module_WireMessageQuery&() {
        return message;
    }

    bool send(uint8_t address);
    size_t write(uint8_t *buffer, size_t size);

};

class ReplyMessage : public MessageWrapper {
private:
    fk_module_WireMessageReply message = fk_module_WireMessageReply_init_default;
    Pool *pool;

public:
    ReplyMessage(Pool *pool) : MessageWrapper(fk_module_WireMessageReply_fields), pool(pool) {
    }

    fk_module_WireMessageReply &m() {
        return message;
    }

    operator fk_module_WireMessageReply&() {
        return message;
    }

    bool receive(uint8_t address);
    size_t write(uint8_t *buffer, size_t size);

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

private:
    bool write(const pb_field_t *fields, void *src);
    bool read(const pb_field_t *fields, void *src);

};

}

#endif
