#pragma once

#include <utility>

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
    void clear() {
        for (size_t i = 0; i < Size; ++i) {
            ((uint8_t *)buffer)[i] = 0;
        }
    }

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
    int32_t size() {
        return position;
    }

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

public:
    int32_t available() {
        return buffer.size - position;
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
        if (bytes < 0) {
            return bytes;
        }

        auto stream = pb_istream_from_buffer(buffer, bytes);
        if (!pb_decode_delimited(&stream, fields, message)) {
            return Stream::EOS;
        }

        return bytes;
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

class StreamCopier {
private:
    BufferPtr buffer;
    size_t position{ 0 };

public:
    StreamCopier(BufferPtr &&bp) : buffer(std::forward<BufferPtr>(bp)) {
    }

public:
    int32_t copy(Reader &reader, Writer &writer);

};

}

#include "ring_buffers.h"
#include "varint_streams.h"
