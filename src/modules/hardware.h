#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <cinttypes>

namespace fk {

class ModuleHardware;

class SpiEnabler {
private:
    ModuleHardware *hardware_;

public:
    SpiEnabler(ModuleHardware *hardware);
    ~SpiEnabler();

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
    bool has_been_on_for(uint32_t time) const;
    bool take();
    bool release();
    bool task();
    bool touch();

};

class ModuleHardware {
private:
    PowerSwitch spi_power_;

public:
    friend class FlashEnabler;

public:
    void task();

public:
    SpiEnabler enable_spi();
    bool has_flash();
    uint8_t flash_cs();
    void spi_take();
    void spi_release();

};

}

#endif
