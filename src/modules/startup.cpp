#include "startup.h"
#include "module_idle.h"
#include "module_callbacks.h"
#include "two_wire_child.h"
#include "rtc.h"
#include "hardware.h"

namespace fk {

void Booting::task() {
    services().leds->setup();
    services().watchdog->setup();

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

        if (!flashFs->initialize(hw->flash, 2048)) {
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
    }
    else {
        log("Flash unconfigured.");
    }
}

}
