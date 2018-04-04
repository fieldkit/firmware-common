#pragma once

#include "protobuf.h"
#include "debug.h"

namespace fk {

class BufferPtr {
public:
    uint8_t *ptr;
    size_t size;
    size_t index;

public:
    BufferPtr(uint8_t *ptr, size_t size) : ptr(ptr), size(size), index(0) { }

public:
    bool empty() {
        return index == 0;
    }

};

class BufferAllocator {
public:
    virtual BufferPtr allocate(size_t size) = 0;

};

template<size_t Size>
class AlignedStorageBuffer {
private:
    typename std::aligned_storage<sizeof(uint8_t), alignof(uint8_t)>::type buffer[Size];

public:
    BufferPtr toBufferPtr() {
        return BufferPtr{ (uint8_t *)buffer, Size };
    }

};

class Writer {
public:
    virtual void beginning() = 0;
    virtual int32_t write(uint8_t *ptr, size_t size) = 0;
    virtual int32_t write(uint8_t byte) = 0;
    virtual size_t remaining() = 0;

};

class Reader {
public:
    static constexpr int32_t EOS = -1;

public:
    virtual void beginning() = 0;
    virtual int32_t available() = 0;
    virtual int32_t read() = 0;

public:
    virtual int32_t read(uint8_t *ptr, size_t size) {
        for (size_t i = 0; i < size; ++i) {
            auto r = read();
            if (r < 0) {
                if (i == 0) {
                    return -1;
                }
                return i;
            }
            ptr[i] = r;
        }
        return size;
    }

};

class DirectWriter : public Writer {
protected:
    BufferPtr buffer;

public:
    DirectWriter(BufferPtr buffer) : buffer(buffer) {
    }

public:
    virtual void beginning() override {
        buffer.index = 0;
    }

    virtual size_t remaining() override {
        return 0;
    }

    virtual int32_t write(uint8_t *ptr, size_t size) override {
        if (buffer.index + size >= buffer.size) {
            return -1;
        }
        memcpy(buffer.ptr + buffer.index, ptr, size);
        buffer.index += size;
        return size;
    }

    virtual int32_t write(uint8_t byte) override {
        if (buffer.index >= buffer.size) {
            return -1;
        }
        buffer.ptr[buffer.index++] = byte;
        return 0;
    }

public:
    int32_t write(const char *str) {
        return write((uint8_t *)str, strlen(str));
    }

    BufferPtr toBufferPtr() {
        return BufferPtr{ buffer.ptr, buffer.index };
    }

};

class DirectReader : public Reader {
private:
    BufferPtr buffer;

public:
    DirectReader(BufferPtr buffer) : buffer(buffer) {
    }

public:
    void beginning() override {
        buffer.index = 0;
    }

    int32_t available() override {
        return buffer.size - buffer.index;
    }

    int32_t read() override {
        if (buffer.index >= buffer.size) {
            return EOS;
        }
        return buffer.ptr[buffer.index++];
    }

    using Reader::read;

};

class ProtoBufMessageWriter : public DirectWriter {
public:
    ProtoBufMessageWriter(BufferPtr buffer) : DirectWriter(buffer) {
    }

    int32_t write(const pb_field_t *fields, void *message) {
        size_t required = 0;

        if (!pb_get_encoded_size(&required, fields, message)) {
            return 0;
        }

        auto stream = pb_ostream_from_buffer((uint8_t *)buffer.ptr, buffer.size);
        if (!pb_encode_delimited(&stream, fields, message)) {
            return 0;
        }

        buffer.index += stream.bytes_written;

        return stream.bytes_written;
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
    void beginning() override {
        for (auto i = 0; i < 2; ++i) {
            if (readers[i] != nullptr) {
                readers[i]->beginning();
            }
        }
    }

    int32_t available() override {
        int32_t total = 0;
        bool hasReaders = false;
        for (auto i = 0; i < 2; ++i) {
            if (readers[i] != nullptr) {
                total += readers[i]->available();
                hasReaders = true;
            }
        }
        return hasReaders ? total : EOS;
    }

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
        return -1;
    }

};

}
