#include "startup.h"
#include "module_idle.h"
#include "module_callbacks.h"
#include "two_wire_child.h"
#include "rtc.h"
#include "hardware.h"
#include "tuning.h"

namespace fk {

void Booting::task() {
    services().leds->setup();
    services().watchdog->setup();

    log("Hash(%s)", firmware_version_get());
    log("Build(%s)", firmware_build_get());

    clock.begin();

    setupFlash();

    services().child->setup();

    transit(services().callbacks->states().configure);
}

void Booting::setupFlash() {
    auto hw = services().hardware;

    if (hw->flash > 0) {
        auto flashFs = services().flashFs;
        auto flashState = services().flashState;

        if (hw->flash_enable > 0) {
            digitalWrite(hw->flash_enable, HIGH);
        }

        if (!flashFs->initialize(hw->flash, SuperBlockSize)) {
            log("Flash unavailable.");
            fk_assert(false);
        }

        if (!flashState->initialize()) {
            log("Flash initialize failed.");
            fk_assert(false);
        }

        if (!flashFs->reclaim(*flashState)) {
            log("Flash reclaim failed.");
            fk_assert(false);
        }

        if (hw->flash_enable > 0) {
            digitalWrite(hw->flash_enable, LOW);
        }
    }
    else {
        log("Flash unconfigured.");
    }
}

}
