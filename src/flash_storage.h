#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

#include <alogging/alogging.h>
#include <phylum/super_block.h>
#include <backends/arduino_serial_flash/arduino_serial_flash.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "debug.h"

namespace fk {

constexpr const char LogName[] = "Flash";

using Logger = SimpleLog<LogName>;

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
    T& state() {
        return manager_.state();
    }

    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512) {
        if (!storage_.initialize(cs, sector_size)) {
            Logger::error("Initialize failed");
            return false;
        }

        if (!storage_.open()) {
            Logger::error("Open failed");
            return false;
        }

        if (!manager_.locate()) {
            Logger::info("Erasing");
            if (!erase()) {
                Logger::error("Erase failed");
                return false;
            }
        }
        else {
            Logger::info("Located");
        }

        if (!allocator_.initialize()) {
            Logger::error("Initialize failed");
            return false;
        }

        return true;
    }

    bool erase() {
        if (!storage_.erase()) {
            return false;
        }

        if (!manager_.create()) {
            return false;
        }

        if (!manager_.locate()) {
            return false;
        }

        return true;
    }

    bool save() {
        if (!manager_.save()) {
            return false;
        }

        auto location = manager_.location();
        Logger::info("Saved (%lu:%d)", location.block, location.sector);
        return true;
    }

};

#endif // FK_DISABLE_FLASH

}

#endif // FK_FLASH_STORAGE_H_INCLUDED
