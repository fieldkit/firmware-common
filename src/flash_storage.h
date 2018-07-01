#ifndef FK_FLASH_STORAGE_H_INCLUDED
#define FK_FLASH_STORAGE_H_INCLUDED

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

    bool initialize(uint8_t cs) {
        return true;
    }

};

#else

template<typename T>
class FlashStorage {
private:
    using FlashStateManager = phylum::SerialFlashStateManager<T>;
    phylum::ArduinoSerialFlashBackend storage_;
    phylum::SerialFlashAllocator allocator_{ storage_ };
    FlashStateManager manager_{ storage_, allocator_ };

public:
    T& state() {
        return manager_.state();
    }

    bool save() {
        if (!manager_.save()) {
            return false;
        }
        log("Saved");
        return true;
    }

    bool initialize(uint8_t cs) {
        if (!storage_.initialize(cs)) {
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
