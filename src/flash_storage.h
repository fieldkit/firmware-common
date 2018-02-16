#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

#include <Arduino.h>
#include <SerialFlash.h>

namespace fk {

class FlashStorage {
private:
    SerialFlashChip *serialFlash;
    uint32_t blockSize{ 0 };
    uint32_t block{ 0 };

public:
    FlashStorage(SerialFlashChip &serialFlash);

public:
    bool setup();

public:
    size_t write(void *ptr, size_t size);
    size_t read(void *ptr, size_t size);
    size_t write(uint32_t address, void *ptr, size_t size);
    size_t read(uint32_t address, void *ptr, size_t size);

};

}

#endif
