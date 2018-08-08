#include "module_idle.h"

#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"

namespace fk {

void ModuleIdle::react(ModuleQueryEvent const &mqe) {
    transit<ModuleServicer>();
}

void ModuleIdle::task() {
    services().watchdog->task();
    delay(10);
}

}
