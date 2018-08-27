#include "startup.h"
#include "module_idle.h"
#include "two_wire_child.h"
#include "rtc.h"

namespace fk {

Booting::Deferred Booting::configure_;

void Booting::task() {
    services().leds->setup();
    services().watchdog->setup();

    clock.begin();

    setupFlash();

    services().child->setup();

    transit<ModuleIdle>();
}

void Booting::setupFlash() {
    auto flashFs = services().flashFs;
    auto flashState = services().flashState;

    if (!flashFs->initialize(6)) {
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

}
