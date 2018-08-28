#ifndef FK_HARDWARE_H_INCLUDED
#define FK_HARDWARE_H_INCLUDED

#include <cinttypes>

namespace fk {

struct ModuleHardware {
    uint8_t flash{ 0 };

    ModuleHardware() {
    }

    ModuleHardware(uint8_t flash) : flash(flash) {
    }
};

}

#endif
