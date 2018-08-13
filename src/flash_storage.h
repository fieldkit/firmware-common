#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

#include <alogging/alogging.h>
#include <phylum/super_block.h>
#include <phylum/serial_flash_state_manager.h>
#include <phylum/serial_flash_fs.h>
#include <backends/arduino_serial_flash/arduino_serial_flash.h>
#include <backends/arduino_serial_flash/serial_flash_allocator.h>

#include "debug.h"
#include "watchdog.h"
#include "module_info.h"

namespace fk {

constexpr const char FlashStorageLogName[] = "Flash";

using FlashLog = SimpleLog<FlashStorageLogName>;

template<typename T>
class FlashState;

class SerialFlashFileSystem : public phylum::StorageBackendCallbacks {
private:
    Watchdog *watchdog_;
    phylum::ArduinoSerialFlashBackend storage_;
    phylum::SerialFlashAllocator allocator_{ storage_ };
    phylum::Files files_{ &storage_, &allocator_ };

public:
    SerialFlashFileSystem(Watchdog &watchdog) : watchdog_(&watchdog), storage_(*this) {
    }

public:
    phylum::Files &files() {
        return files_;
    }

public:
    bool initialize(uint8_t cs, phylum::sector_index_t sector_size = 512);

    bool erase();

    template<typename T>
    bool reclaim(FlashState<T> &manager) {
        phylum::UnusedBlockReclaimer reclaimer(&files_, &manager.manager());
        reclaim(reclaimer, manager.state());
        return reclaimer.reclaim();
    }

    template<typename T>
    friend class FlashState;

protected:
    bool busy(uint32_t elapsed) override;

    bool reclaim(phylum::UnusedBlockReclaimer &reclaimer, PersistedState &state);

};

template<typename T>
class FlashState {
private:
    SerialFlashFileSystem *flashFs_;
    phylum::SerialFlashStateManager<T> manager_;

public:
    FlashState(SerialFlashFileSystem &flashFs) : flashFs_(&flashFs), manager_{ flashFs.storage_, flashFs.allocator_ } {
    }

public:
    phylum::SerialFlashStateManager<T>& manager() {
        return manager_;
    }

    T& state() {
        return manager_.state();
    }

    bool initialize() {
        if (!manager_.locate()) {
            if (!erase()) {
                FlashLog::error("Erase failed");
                return false;
            }
        }
        else {
            auto location = manager_.location();
            FlashLog::info("Located (%lu.%d) (size = %d)", location.block, location.sector, sizeof(T));
        }

        return true;
    }

    bool erase() {
        FlashLog::info("Creating (size = %d)", sizeof(T));
        if (!manager_.create()) {
            return false;
        }

        FlashLog::info("Locating");
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
        FlashLog::info("Saved (%lu:%d)", location.block, location.sector);
        return true;
    }

};

}

#endif // FK_FLASH_STORAGE_H_INCLUDED
