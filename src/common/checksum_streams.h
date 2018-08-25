#ifndef FK_CHECKSUM_STREAMS_H_INCLUDED
#define FK_CHECKSUM_STREAMS_H_INCLUDED

#include <lwstreams/lwstreams.h>

namespace fk {

class Crc32Reader : public lws::Reader {
private:
    lws::Reader *target_;
    uint32_t crc_{ ~uint32_t(0) };

public:
    Crc32Reader(lws::Reader &target) : target_(&target) {
    }

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;

public:
    uint32_t checksum() const {
        return crc_;
    }
};

class Crc32Writer : public lws::Writer {
private:
    lws::Writer *target_;
    uint32_t crc_{ ~uint32_t(0) };

public:
    Crc32Writer(lws::Writer &target) : target_(&target) {
    }

public:
    int32_t write(uint8_t *ptr, size_t size) override;
    int32_t write(uint8_t byte) override;
    void close() override;

public:
    uint32_t checksum() const {
        return crc_;
    }
};

}

#endif
