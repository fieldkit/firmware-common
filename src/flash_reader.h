#ifndef FK_FLASH_READER_H_INCLUDED
#define FK_FLASH_READER_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "flash_storage.h"

namespace fk {

using FlashBackend = SerialFlashChip;

class FlashWriter : public lws::Writer {
private:
    FlashBackend *flash_;
    uint32_t offset_{ 0 };
    uint32_t position_{ 0 };
    bool closed_{ false };

public:
    FlashWriter(FlashBackend *flash, uint32_t offset);

public:
    int32_t write(uint8_t *ptr, size_t size) override;
    int32_t write(uint8_t byte) override;
    void close() override;

};

class FlashReader : public lws::SizedReader {
private:
    FlashBackend *flash_;
    uint32_t offset_{ 0 };
    uint32_t position_{ 0 };
    uint32_t length_{ 0 };
    bool closed_{ false };

public:
    FlashReader(FlashBackend *flash, uint32_t offset, uint32_t length);

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;
    size_t size() override;

};

}

#endif
