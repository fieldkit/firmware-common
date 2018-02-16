#ifndef FK_MESSAGE_BUFFER_H_INCLUDED
#define FK_MESSAGE_BUFFER_H_INCLUDED

#include <fk-app-protocol.h>
#include <fk-module-protocol.h>

#include "two_wire.h"
#include "app_messages.h"
#include "module_messages.h"
#include "data_messages.h"

namespace fk {

class MessageBuffer {
protected:
    size_t pos{ 0 };

public:
    virtual uint8_t *ptr() = 0;

    virtual size_t size() = 0;

    bool write(ModuleQueryMessage &message);

    bool write(ModuleReplyMessage &message);

    bool read(ModuleQueryMessage &message);

    bool read(ModuleReplyMessage &message);

    bool read(AppQueryMessage &message);

    bool write(AppReplyMessage &message);

    bool write(DataRecordMessage &message);

    size_t position() {
        return pos;
    }

    void clear() {
        pos = 0;
    }

    bool empty() {
        return pos == 0;
    }

    void move(size_t p) {
        pos = p;
    }

public:
    virtual size_t read() {
        return 0;
    }

    virtual size_t write() {
        return 0;
    }

private:
    bool write(const pb_field_t *fields, void *src);
    bool read(const pb_field_t *fields, void *src);

};

template<size_t Size>
class ArrayMessageBuffer : public MessageBuffer {
protected:
    uint8_t buffer[Size];

public:
    uint8_t *ptr() override {
        return buffer;
    }

    size_t size() override {
        return sizeof(buffer);
    }

};

class TwoWireMessageBuffer : public ArrayMessageBuffer<256> {
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
