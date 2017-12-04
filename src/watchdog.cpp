#include <Arduino.h>

#include "watchdog.h"
#include "debug.h"

namespace fk {

constexpr uint32_t Interval = 5000;
constexpr const char Log[] = "Watchdog";

void Watchdog::tick() {
    if (millis() > time) {
        time = millis() + Interval;
        debugfpln(Log, "Tick (%d free)", fk_free_memory());
    }
}

}
