#include "module_idle.h"

#include "tuning.h"
#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"
#include "message_buffer.h"
#include "two_wire_child.h"

namespace fk {

void ModuleIdle::entry() {
    ModuleServicesState::entry();

    services().watchdog->idling();
}

void ModuleIdle::task() {
    if (!services().child->incoming().empty()) {
        transit<ModuleServicer>();
    }
    else {
        services().alive();
    }

    if (elapsed() > ModuleIdleRebootInterval) {
        log("Reboot due to inactivity.");
        NVIC_SystemReset();
    }
}

}
