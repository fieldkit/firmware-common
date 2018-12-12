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

class PowerSwitch {
private:
    uint8_t refs_{ 0 };
    uint32_t time_on_{ 0 };
    uint32_t time_off_{ 0 };
    uint16_t minimum_on_{ 0 };

public:
    PowerSwitch(uint16_t minimum_on = 0) : minimum_on_(minimum_on) {
    }

public:
    bool take();
    bool release();
    bool task();

};

class ModuleHardware {
private:
    PowerSwitch spi_power_;

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
