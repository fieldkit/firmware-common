#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <cinttypes>

namespace fk {

class ModuleHardware;

class FlashEnabler {
private:
    ModuleHardware *hardware_;

public:
    FlashEnabler(ModuleHardware *hardware);
    ~FlashEnabler();

};

class ModuleHardware {
private:
    uint8_t flash_refs_{ 0 };
    uint32_t minimum_enable_time_{ 0 };
    uint32_t flash_on_{ 0 };
    uint32_t flash_off_{ 0 };

public:
    uint8_t flash{ 0 };
    uint8_t flash_enable{ 0 };

public:
    ModuleHardware() {
    }

    ModuleHardware(uint8_t flash) : flash(flash) {
    }

    ModuleHardware(uint8_t flash, uint8_t flash_enable, uint32_t minimum_enable_time = 0) : minimum_enable_time_(minimum_enable_time), flash(flash), flash_enable(flash_enable) {
    }

    friend class FlashEnabler;

public:
    void task();

public:
    FlashEnabler enable_flash() {
        return { this };
    }

    void flash_take();
    void flash_release();

};

}

#endif
