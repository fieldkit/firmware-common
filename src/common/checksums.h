#ifndef FK_CHECKSUMS_H_INCLUDED
#define FK_CHECKSUMS_H_INCLUDED

#include <cinttypes>
#include <cstdio>

namespace fk {

uint32_t crc32_update(uint32_t crc, uint8_t data);

uint32_t crc32_checksum(uint8_t *data, size_t size);

}

#endif
