#include "module_idle.h"

#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"
#include "message_buffer.h"

namespace fk {

void ModuleIdle::task() {
    if (!services().incoming->empty()) {
        transit<ModuleServicer>();
    }
    else {
        services().watchdog->task();
        delay(10);
    }
}

}
