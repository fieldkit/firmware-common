#include "flash_storage.h"
#include "debug.h"

namespace fk {

FlashStorage::FlashStorage(SerialFlashChip &serialFlash) : serialFlash(&serialFlash) {
}

static uint32_t crc16_table[16] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

static uint16_t crc16_update(uint16_t start, uint8_t *p, size_t n) {
    uint16_t crc = start;
    uint16_t r;

    while (n-- > 0) {
        /* compute checksum of lower four bits of *p */
        r = crc16_table[crc & 0xF];
        crc = (crc >> 4) & 0x0FFF;
        crc = crc ^ r ^ crc16_table[*p & 0xF];

        /* now compute checksum of upper four bits of *p */
        r = crc16_table[crc & 0xF];
        crc = (crc >> 4) & 0x0FFF;
        crc = crc ^ r ^ crc16_table[(*p >> 4) & 0xF];

        p++;
    }

    return crc;
}

bool FlashStorage::setup() {
    blockSize = serialFlash->blockSize();
    return blockSize > 0;
}

size_t FlashStorage::write(void *ptr, size_t size) {
    block = (block + 1) % 2;
    return write(block * blockSize, ptr, size);
}

size_t FlashStorage::read(void *ptr, size_t size) {
    if (read(block * blockSize, ptr, size) != size) {
        block = (block + 1) % 2;
        if (read(block * blockSize, ptr, size) != size) {
            return 0;
        }
    }
    return size;
}

size_t FlashStorage::write(uint32_t address, void *ptr, size_t size) {
    uint8_t buffer[size + sizeof(uint16_t)];

    uint16_t oldHash{ 0 };
    serialFlash->read(address, (uint8_t *)&oldHash, sizeof(uint16_t));

    auto newHash = crc16_update(address, (uint8_t *)ptr, size);
    if (newHash == oldHash) {
        return size;
    }

    memcpy(buffer, &newHash, sizeof(uint16_t));
    memcpy(buffer + sizeof(uint16_t), ptr, size);

    debugfpln("Flash", "Writing %lu (%d)...", address, newHash);
    serialFlash->eraseBlock(address);
    serialFlash->write(address, buffer, sizeof(buffer));

    return size;
}

size_t FlashStorage::read(uint32_t address, void *ptr, size_t size) {
    uint8_t buffer[size + sizeof(uint16_t)];
    uint16_t hash{ 0 };

    serialFlash->read(address, buffer, sizeof(buffer));

    memcpy(&hash, buffer, sizeof(uint16_t));

    auto expected = crc16_update(address, buffer + sizeof(uint16_t), size);
    if (expected != hash) {
        debugfpln("Flash", "Hash mismatch (%d != %d)", expected, hash);
        return 0;
    }

    memcpy(ptr, buffer + sizeof(uint16_t), size);
    return size;
}

}
