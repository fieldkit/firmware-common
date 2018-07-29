#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

#include <alogging/alogging.h>
#include <phylum/super_block.h>
#include <backends/arduino_serial_flash/arduino_serial_flash.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "debug.h"
#include "watchdog.h"

namespace fk {

constexpr const char FlashStorageLogName[] = "Flash";

#ifdef FK_DISABLE_FLASH

template<typename T>
class FlashStorage {
private:
    T state_;

public:
    FlashStorage(Watchdog &watchdog) {
    }

    T& state() {
        return state_;
    }

    bool save() {
        return true;
    }

    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512) {
        return true;
    }

    bool erase() {
        return true;
    }

};

#else

template<typename T>
class FlashStorage : public phylum::StorageBackendCallbacks {
private:
    Watchdog *watchdog_;
    phylum::ArduinoSerialFlashBackend storage_;
    phylum::SerialFlashAllocator allocator_{ storage_ };
    phylum::SerialFlashStateManager<T> manager_{ storage_, allocator_ };

    using Logger = SimpleLog<FlashStorageLogName>;

public:
    FlashStorage(Watchdog &watchdog) : watchdog_(&watchdog), storage_(*this) {
    }

public:
    T& state() {
        return manager_.state();
    }

    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512) {
        if (!storage_.initialize(cs, sector_size)) {
            Logger::error("Initialize failed");
            return false;
        }

        auto g = storage_.geometry();
        auto firmwareArea = 256 * 1024 * 2;
        auto firmwareBlocks = firmwareArea / g.block_size();
        g.number_of_blocks -= firmwareBlocks;
        storage_.geometry(g);

        Logger::info("Flash Geometry: (%d x %lu) (%d for firmware)",
                     g.number_of_blocks, g.block_size(), firmwareBlocks);

        if (!storage_.open()) {
            Logger::error("Open failed");
            return false;
        }

        if (!manager_.locate()) {
            if (!erase()) {
                Logger::error("Erase failed");
                return false;
            }
        }
        else {
            auto location = manager_.location();
            Logger::info("Located (%d.%d)", location.block, location.sector);
        }

        if (!allocator_.initialize()) {
            Logger::error("Initialize failed");
            return false;
        }

        return true;
    }

    bool erase() {
        Logger::info("Erasing");
        if (!storage_.erase()) {
            return false;
        }

        Logger::info("Creating");
        if (!manager_.create()) {
            return false;
        }

        Logger::info("Locating");
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

protected:
    bool busy(uint32_t elapsed) override {
        watchdog_->task();

        return true;
    }

};

#endif // FK_DISABLE_FLASH

}

#endif // FK_FLASH_STORAGE_H_INCLUDED
