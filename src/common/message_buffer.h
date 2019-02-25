#ifndef FK_MESSAGE_BUFFER_H_INCLUDED
#define FK_MESSAGE_BUFFER_H_INCLUDED

#include "two_wire.h"

namespace fk {

class MessageBuffer {
protected:
    size_t pos{ 0 };

public:
    virtual uint8_t *ptr() = 0;

    virtual size_t size() = 0;

    template<typename T>
    bool read(T &message) {
        return read(T::fields, message.forDecode());
    }

    template<typename T>
    bool write(T &message) {
        return write(T::fields, message.forEncode());
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
    bool write(const pb_msgdesc_t *fields, void *src);
    bool read(const pb_msgdesc_t *fields, void *src);

};

class DirectMessageBuffer : public MessageBuffer {
protected:
    uint8_t *buffer;
    size_t length;

public:
    DirectMessageBuffer(uint8_t *buffer, size_t length) : buffer(buffer), length(length) {
    }

public:
    void end() {
        pos = length;
    }

    uint8_t *ptr() override {
        return buffer;
    }

    size_t size() override {
        return length;
    }

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
