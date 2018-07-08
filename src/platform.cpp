#include "platform.h"

#include <Arduino.h>

namespace fk {

uint32_t fk_uptime() {
    return millis();
}

bool fk_console_attached() {
    return Serial;
}

}
