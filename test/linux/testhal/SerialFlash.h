#pragma once

class SerialFlashChip {
public:
    bool begin(uint8_t cs) {
        return false;
    }

    void readID(uint8_t *ptr) {
    }

    uint32_t capacity(uint8_t *ptr) {
        return 0;
    }

    uint32_t blockSize() {
        return 0;
    }

    void eraseAll() {
    }

    void eraseBlock(uint32_t address) {
    }

    void read(uint32_t address, void *ptr, size_t n) {
    }

    void write(uint32_t address, void *ptr, size_t n) {
    }

    bool ready() {
        return true;
    }
};
