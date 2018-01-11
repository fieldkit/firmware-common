#ifndef FK_MESSAGE_BUFFER_H_INCLUDED
#define FK_MESSAGE_BUFFER_H_INCLUDED

#include <fk-app-protocol.h>
#include <fk-module-protocol.h>

#include "i2c.h"
#include "app_messages.h"
#include "module_messages.h"

namespace fk {

class MessageBuffer {
protected:
    uint8_t buffer[4096 + 64];
    size_t pos{ 0 };

public:
    uint8_t *ptr() {
        return buffer;
    }

    size_t size() {
        return sizeof(buffer);
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

    bool write(AppReplyMessage &message);

private:
    bool write(const pb_field_t *fields, void *src);
    bool read(const pb_field_t *fields, void *src);

public:
    virtual size_t read() {
        return 0;
    }

    virtual size_t write() {
        return 0;
    }

};

class TwoWireMessageBuffer : public MessageBuffer {
private:
    TwoWireBus *bus;

public:
    TwoWireMessageBuffer(TwoWireBus &bus) : bus(&bus) {
    }

public:
    bool send(uint8_t address);
    bool receive(uint8_t address);
    bool readIncoming(size_t bytes);

};

}

#endif
