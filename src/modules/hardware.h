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
    uint32_t flash_on_{ 0 };
    uint32_t flash_off_{ 0 };

public:
    friend class FlashEnabler;

public:
    void task();

public:
    FlashEnabler enable_flash();
    bool has_flash();
    uint8_t flash_cs();
    void flash_take();
    void flash_release();

};

}

#endif
