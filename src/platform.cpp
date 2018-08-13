#include "platform.h"

#include <Arduino.h>

namespace fk {

static uint32_t offset{ 0 };

static bool console_attached{ false };

uint32_t fk_uptime() {
    return millis() + offset;
}

uint32_t fk_uptime_adjust(uint32_t by) {
    offset += by;
    return offset;
}

bool fk_console_attached() {
    if (Serial) {
        console_attached = true;
    }
    /*
    return console_attached;
     */
    return true;
}

bool fk_logging_trace_enabled() {
    return fk_console_attached();
}

}
