#include <algorithm>

#include "flash_reader.h"
#include "flash_storage.h"

namespace fk {

FlashWriter::FlashWriter(FlashBackend *flash, uint32_t offset) : flash_(flash), offset_(offset) {
}

int32_t FlashWriter::write(uint8_t *ptr, size_t size) {
    if (closed_) {
        return EOS;
    }

    flash_->write(offset_ + position_, ptr, size);

    position_ += size;

    return 0;
}

int32_t FlashWriter::write(uint8_t byte) {
    fk_assert(false);
    return 0;
}

void FlashWriter::close() {
    closed_ = true;
}

FlashReader::FlashReader(FlashBackend *flash, uint32_t offset, uint32_t length) : flash_(flash), offset_(offset), length_(length) {
}

int32_t FlashReader::read(uint8_t *ptr, size_t size) {
    if (closed_) {
        return EOS;
    }

    auto remaining = length_ - position_;
    auto reading = std::min((uint32_t)size, remaining);

    flash_->read(offset_ + position_, ptr, reading);

    position_ += reading;

    return reading;
}

void FlashReader::close() {
    closed_ = true;
}

size_t FlashReader::size() {
    return length_;
}

int32_t FlashReader::read() {
    fk_assert(false);
    return 0;
}

}
