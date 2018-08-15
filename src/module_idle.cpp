#include "module_idle.h"

#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"
#include "message_buffer.h"
#include "tuning.h"

#include <Arduino.h>

namespace fk {

void ModuleIdle::task() {
    if (fk_uptime() - tick_ > 5000) {
        trace("Tick");
        tick_ = fk_uptime();
    }
    if (!services().incoming->empty()) {
        transit<ModuleServicer>();
    }
    else {
        services().watchdog->task();
        delay(10);
    }

    if (elapsed() > ModuleIdleRebootInterval) {
        log("Reboot due to inactivity.");
        NVIC_SystemReset();
    }
}

}
