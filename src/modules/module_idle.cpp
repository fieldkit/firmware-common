#include "module_idle.h"

#include "tuning.h"
#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"
#include "message_buffer.h"
#include "two_wire_child.h"

#include <Arduino.h>

namespace fk {

void ModuleIdle::entry() {
    ModuleServicesState::entry();

    services().watchdog->idling();
}

void ModuleIdle::task() {
    if (fk_uptime() - tick_ > 5000) {
        trace("Tick");
        tick_ = fk_uptime();
    }

    if (!services().child->incoming().empty()) {
        transit<ModuleServicer>();
    }
    else {
        services().watchdog->task();
    }

    if (elapsed() > ModuleIdleRebootInterval) {
        log("Reboot due to inactivity.");
        NVIC_SystemReset();
    }
}

}
