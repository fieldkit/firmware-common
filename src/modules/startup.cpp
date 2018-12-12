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

    if (hw->has_flash()) {
        auto enableSpi = services().hardware->enable_spi();
        auto flashFs = services().flashFs;
        auto flashState = services().flashState;

        delay(100);

        if (!flashFs->initialize(hw->flash_cs(), SuperBlockSize)) {
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
