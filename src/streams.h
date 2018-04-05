#pragma once

#include "protobuf.h"
#include "debug.h"

namespace fk {

class BufferPtr {
public:
    uint8_t *ptr{ nullptr };
    size_t size{ 0 };

public:
    BufferPtr(uint8_t *ptr, size_t size) : ptr(ptr), size(size) { }

public:
    uint8_t &operator[](int32_t index) {
        fk_assert(index >= 0 && index < (int32_t)size);
        return ptr[index];
    }
};

template<size_t Size>
class AlignedStorageBuffer {
private:
    typename std::aligned_storage<sizeof(uint8_t), alignof(uint8_t)>::type buffer[Size];

public:
    const size_t size{ Size };

public:
    BufferPtr toBufferPtr() {
        return BufferPtr{ (uint8_t *)buffer, Size };
    }

public:
    uint8_t &operator[](int32_t index) {
        fk_assert(index >= 0 && index < (int32_t)Size);
        return ((uint8_t *)buffer)[index];
    }
};

class Stream {
public:
    static constexpr int32_t EOS = -1;

};

class Writer : public Stream {
public:
    virtual int32_t write(uint8_t *ptr, size_t size) = 0;
    virtual int32_t write(uint8_t byte) = 0;
    virtual void close() = 0;

public:
    int32_t write(const char *str) {
        return write((uint8_t *)str, strlen(str));
    }
};

class Reader : public Stream {
public:
    virtual int32_t read() = 0;
    virtual void close() = 0;

public:
    virtual int32_t read(uint8_t *ptr, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            auto r = read();
            if (r < 0) {
                if (i == 0) {
                    return EOS;
                }
                return i;
            }
            ptr[i] = r;
        }
        return size;
    }
};

class DirectWriter : public Writer {
private:
    BufferPtr buffer;
    int32_t position{ 0 };

public:
    DirectWriter(BufferPtr buffer) : buffer(buffer) {
    }

public:
    using Writer::write;

    int32_t write(uint8_t *ptr, size_t size) override {
        if (position == EOS) {
            return EOS;
        }
        auto available = buffer.size - position;
        auto copying = size > available ? available : size;
        if (copying > 0) {
            memcpy(buffer.ptr + position, ptr, copying);
            position += copying;
        }
        return copying;
    }

    int32_t write(uint8_t byte) override {
        if (position == EOS) {
            return EOS;
        }
        if (position == (int32_t)buffer.size) {
            return EOS;
        }
        buffer.ptr[position++] = byte;
        return 1;
    }

    void close() override {
        position = EOS;
    }

public:
    int32_t available() {
        return buffer.size - position;
    }

    BufferPtr toBufferPtr() {
        fk_assert(position >= 0);
        return BufferPtr{ buffer.ptr, (size_t)position };
    }

protected:
    uint8_t *ptr() {
        return buffer.ptr + position;
    }

    int32_t seek(int32_t bytes) {
        position += bytes;
        return position;
    }

};

class DirectReader : public Reader {
private:
    BufferPtr buffer;
    int32_t position{ 0 };

public:
    DirectReader(BufferPtr buffer) : buffer(buffer) {
    }

public:
    using Reader::read;

    int32_t read() override {
        if (position == EOS) {
            return EOS;
        }
        if (position >= (int32_t)buffer.size) {
            return EOS;
        }
        return buffer.ptr[position++];
    }

    void close() override {
        position = EOS;
    }

};

class BufferedReader : public Reader {
private:
    BufferPtr buffer;
    int32_t position{ 0 };
    int32_t buffered{ 0 };

public:
    BufferedReader(BufferPtr buffer) : buffer(buffer) {
    }

public:
    using Reader::read;

    int32_t read() override {
        if (replenish() < 0) {
            return EOS;
        }
        if (position >= (int32_t)buffer.size) {
            return EOS;
        }
        return buffer.ptr[position++];
    }

    void close() override {
    }

protected:
    virtual int32_t fill(BufferPtr &buffer) = 0;

    int32_t replenish() {
        if (buffered == 0 || buffered == position) {
            buffered = fill(buffer);
            position = 0;
        }
        if (buffered < 0) {
            return EOS;
        }
        return buffered;
    }

};

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

template<typename RingBufferType>
class CircularStreams {
    using OuterType = CircularStreams<RingBufferType>;

    class RingReader : public Reader {
    private:
        OuterType *cs;

    public:
        RingReader(OuterType *cs) : cs(cs) {
        }

    public:
        int32_t read(uint8_t *ptr, size_t size) override {
            if (cs->buffer.empty()) {
                if (cs->closed) {
                    return EOS;
                }
                return 0;
            }

            return Reader::read(ptr, size);
        }

        int32_t read() override {
            // Not totally happy with this. Don't have any way of
            // differentiating between empty and EoS though.
            if (cs->buffer.empty()) {
                return EOS;
            }
            return cs->buffer.shift();
        }

        void close() override {
            cs->closeAll();
        }
    };

    class RingWriter : public Writer {
    private:
        OuterType *cs;

    public:
        RingWriter(OuterType *cs) : cs(cs) {
        }

    public:
        using Writer::write;

        int32_t write(uint8_t *ptr, size_t size) override {
            for (size_t i = 0; i < size; ++i) {
                if (cs->buffer.full()) {
                    return i;
                }
                cs->buffer.push(ptr[i]);
            }
            return size;
        }

        int32_t write(uint8_t byte) override {
            if (cs->buffer.full()) {
                return EOS;
            }
            cs->buffer.push(byte);
            return 1;
        }

        void close() override {
            cs->closeAll();
        }
    };

    RingBufferType buffer;
    RingReader reader{ this };
    RingWriter writer{ this };
    bool closed{ false };

public:
    CircularStreams() {
    }

    CircularStreams(RingBufferType &&buffer) : buffer(std::forward<RingBufferType>(buffer)) {
    }

public:
    void closeAll() {
        closed = true;
    }

    Writer &getWriter() {
        return writer;
    }

    Reader &getReader() {
        return reader;
    }

};

class ProtoBufMessageWriter {
private:
    Writer *target;

public:
    ProtoBufMessageWriter(Writer &target) : target(&target) {
    }

    int32_t write(const pb_field_t *fields, void *message) {
        size_t required = 0;

        if (!pb_get_encoded_size(&required, fields, message)) {
            return 0;
        }

        uint8_t buffer[required + ProtoBufEncodeOverhead];
        auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        if (!pb_encode_delimited(&stream, fields, message)) {
            return 0;
        }

        target->write(buffer, stream.bytes_written);

        return stream.bytes_written;
    }

};

class ProtoBufMessageReader {
private:
    Reader *target;

public:
    ProtoBufMessageReader(Reader &target) : target(&target) {
    }

    template<size_t Size>
    int32_t read(const pb_field_t *fields, void *message) {
        uint8_t buffer[Size];
        auto bytes = target->read(buffer, sizeof(buffer));

        auto stream = pb_istream_from_buffer(buffer, bytes);
        if (!pb_decode_delimited(&stream, fields, message)) {
            return Stream::EOS;
        }

        return bytes;
    }

};

class ConcatenatedReader : public Reader {
private:
    Reader *readers[2];

public:
    ConcatenatedReader(Reader *reader1, Reader *reader2) : readers{ reader1, reader2 } {
    }

    ConcatenatedReader(Reader *readers[2]) : readers{ readers[0], readers[1] } {
    }

public:
    int32_t read() override {
        for (auto i = 0; i < 2; ++i) {
            if (readers[i] != nullptr) {
                auto r = readers[i]->read();
                if (r < 0) {
                    readers[i] = nullptr;
                }
                else {
                    return r;
                }
            }
        }
        return EOS;
    }

    void close() override {
    }

};

}