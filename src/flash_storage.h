#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

#include <alogging/alogging.h>
#include <phylum/super_block.h>
#include <backends/arduino_serial_flash/arduino_serial_flash.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

namespace fk {

#ifdef FK_DISABLE_FLASH

template<typename T>
class FlashStorage {
private:
    T state_;

public:
    T& state() {
        return state_;
    }

    bool save() {
        return true;
    }

    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512) {
        return true;
    }

};

#else

template<typename T>
class FlashStorage {
private:
    phylum::ArduinoSerialFlashBackend storage_;
    phylum::SerialFlashAllocator allocator_{ storage_ };
    phylum::SerialFlashStateManager<T> manager_{ storage_, allocator_ };

public:
    FlashStorage() {
        // static_assert(sizeof(T) <= phylum::SectorSize, "state object should be smaller than SectorSize");
    }

    T& state() {
        return manager_.state();
    }

    bool save() {
        if (!manager_.save()) {
            return false;
        }

        auto location = manager_.location();
        logf(LogLevels::INFO, "CoreState", "Saved (%lu:%d)", location.block, location.sector);
        return true;
    }

    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512) {
        if (!storage_.initialize(cs, sector_size)) {
            return false;
        }

        if (!storage_.open()) {
            return false;
        }

        if (!manager_.locate()) {
            if (!manager_.create()) {
                return false;
            }

            if (!manager_.locate()) {
                return false;
            }
        }

        return true;
    }

public:

};

#endif // FK_DISABLE_FLASH

}

#endif
