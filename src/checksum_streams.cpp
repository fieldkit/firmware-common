#include "checksum_streams.h"
#include "checksums.h"
#include "debug.h"

namespace fk {

int32_t Crc32Reader::read(uint8_t *ptr, size_t size) {
    auto bytes = target_->read(ptr, size);

    for (auto i = 0; i < bytes; ++i) {
        crc_ = crc32_update(crc_, *(ptr++));
    }

    return bytes;
}

int32_t Crc32Reader::read() {
    fk_assert(false);
    return 0;
}

void Crc32Reader::close() {
}

int32_t Crc32Writer::write(uint8_t *ptr, size_t size) {
    auto bytes = target_->write(ptr, size);

    for (auto i = 0; i < bytes; ++i) {
        crc_ = crc32_update(crc_, *(ptr++));
    }

    return bytes;
}

int32_t Crc32Writer::write(uint8_t byte) {
    fk_assert(false);
    return 0;
}

void Crc32Writer::close() {
    target_->close();
}

}
