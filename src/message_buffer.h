#ifndef FK_MESSAGE_BUFFER_H_INCLUDED
#define FK_MESSAGE_BUFFER_H_INCLUDED

#include <fk-module-protocol.h>
#include <fk-app-protocol.h>

#include "module_messages.h"
#include "app_messages.h"

namespace fk {

class MessageBuffer {
private:
    uint8_t buffer[FK_MODULE_PROTOCOL_MAX_MESSAGE];
    size_t pos { 0 };

public:
    uint8_t *ptr() {
        return buffer;
    }

    size_t size() {
        return FK_MODULE_PROTOCOL_MAX_MESSAGE;
    }

    size_t position() {
        return pos;
    }

    void clear() {
        pos = 0;
    }

    bool empty() {
        return pos == 0;
    }

    void append(uint8_t c) {
        buffer[pos++] = c;
    }

    void move(size_t p) {
        pos = p;
    }

    bool write(ModuleQueryMessage &message);

    bool write(ModuleReplyMessage &message);

    bool read(ModuleQueryMessage &message);

    bool read(ModuleReplyMessage &message);

    bool read(AppQueryMessage &message);

    bool send(uint8_t address);

    bool receive(uint8_t address);

    bool read(size_t bytes);

private:
    bool write(const pb_field_t *fields, void *src);
    bool read(const pb_field_t *fields, void *src);

};

}

#endif
