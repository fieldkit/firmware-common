#include "platform.h"

#include <Arduino.h>

namespace fk {

static uint32_t offset{ 0 };

uint32_t fk_uptime() {
    return millis() + offset;
}

uint32_t fk_uptime_adjust(uint32_t by) {
    offset += by;
    return offset;
}

bool fk_console_attached() {
    return Serial;
}

}
