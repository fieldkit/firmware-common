#include <utility>

#pragma once

#include "protobuf.h"
#include "debug.h"

namespace fk {

constexpr bool is_power_of_2(int32_t v) {
    return v && ((v & (v - 1)) == 0);
}

template<typename T>
class RingBufferG {
private:
    T bp;
    volatile uint32_t read{ 0 };
    volatile uint32_t write{ 0 };

public:
    RingBufferG() {
    }

    RingBufferG(T bp) : bp(bp) {
        fk_assert(is_power_of_2(bp.size));
    }

    RingBufferG(T &&bp) : bp(std::forward<T>(bp)) {
        fk_assert(is_power_of_2(bp.size));
    }

    void clear() {
        read = write = 0;
        bp.clear();
    }

    void push(uint8_t c) {
        fk_assert(!full());
        bp[mask(write++)] = c;
    }

    uint8_t shift() {
        fk_assert(!empty());
        return bp[mask(read++)];
    }

    uint32_t available() {
        return bp.size - size();
    }

    uint32_t size() {
        return write - read;
    }

    bool empty() {
        return read == write;
    }

    bool full() {
        return size() == bp.size;
    }

private:
    uint32_t mask(uint32_t i) {
        return i & (bp.size - 1);
    }

};

typedef RingBufferG<BufferPtr> RingBufferPtr;

template<size_t Size>
class RingBufferN : public RingBufferG<AlignedStorageBuffer<Size>> {
public:
    RingBufferN() {
    }
};

template<typename RingBufferType> class RingReader;
template<typename RingBufferType> class RingWriter;

template<typename RingBufferType>
class RingReader : public Reader {
private:
    RingBufferType *buffer;
    RingWriter<RingBufferType> *other;
    bool closed{ false };

    friend class RingWriter<RingBufferType>;

public:
    RingReader(RingBufferType *buffer, RingWriter<RingBufferType> *writer) : buffer(buffer), other(writer) {
    }

public:
    int32_t read(uint8_t *ptr, size_t size) override;

    int32_t read() override;

    void close() override;
};

template<typename RingBufferType>
class RingWriter : public Writer {
private:
    RingBufferType *buffer;
    RingReader<RingBufferType> *other;
    bool closed{ false };

    friend class RingReader<RingBufferType>;

public:
    RingWriter(RingBufferType *buffer, RingReader<RingBufferType> *reader) : buffer(buffer), other(reader) {
    }

public:
    using Writer::write;

    int32_t write(uint8_t *ptr, size_t size) override;

    int32_t write(uint8_t byte) override;

    void close() override;
};

template<typename RingBufferType>
class CircularStreams {
    using OuterType = CircularStreams<RingBufferType>;

    RingBufferType buffer;
    RingReader<RingBufferType> reader{ &buffer, &writer };
    RingWriter<RingBufferType> writer{ &buffer, &reader };

public:
    CircularStreams() {
    }

    CircularStreams(RingBufferType &&buffer) : buffer(std::forward<RingBufferType>(buffer)) {
    }

public:
    Writer &getWriter() {
        return writer;
    }

    Reader &getReader() {
        return reader;
    }

    void clear() {
        buffer.clear();
        reader = RingReader<RingBufferType>{ &buffer, &writer };
        writer = RingWriter<RingBufferType>{ &buffer, &reader };
    }

};

template<typename RingBufferType>
int32_t RingReader<RingBufferType>::read(uint8_t *ptr, size_t size) {
    if (buffer->empty()) {
        if (other->closed) {
            return EOS;
        }
        return 0;
    }

    return Reader::read(ptr, size);
}

template<typename RingBufferType>
int32_t RingReader<RingBufferType>::read() {
    // Not totally happy with this. Don't have any way of
    // differentiating between empty and EoS though.
    if (buffer->empty()) {
        return EOS;
    }
    return buffer->shift();
}

template<typename RingBufferType>
void RingReader<RingBufferType>::close() {
    closed = true;
}

template<typename RingBufferType>
int32_t RingWriter<RingBufferType>::write(uint8_t *ptr, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (buffer->full()) {
            return i;
        }
        buffer->push(ptr[i]);
    }
    return size;
}

template<typename RingBufferType>
int32_t RingWriter<RingBufferType>::write(uint8_t byte) {
    if (buffer->full()) {
        return EOS;
    }
    buffer->push(byte);
    return 1;
}

template<typename RingBufferType>
void RingWriter<RingBufferType>::close() {
    closed = true;
}

}
