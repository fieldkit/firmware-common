#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <cinttypes>

namespace fk {

class PinHolder {
private:
    uint8_t pin;

public:
    PinHolder(uint8_t pin) : pin(pin) {
        if (pin > 0) {
            digitalWrite(pin, HIGH);
        }
    }

    ~PinHolder() {
        if (pin > 0) {
            digitalWrite(pin, LOW);
        }
    }

};

struct ModuleHardware {
    uint8_t flash{ 0 };
    uint8_t flash_enable{ 0 };

    ModuleHardware() {
    }

    ModuleHardware(uint8_t flash) : flash(flash) {
    }

    ModuleHardware(uint8_t flash, uint8_t flash_enable) : flash(flash), flash_enable(flash_enable) {
    }

    PinHolder enable_flash() {
        return { flash_enable };
    }
};

}

#endif
