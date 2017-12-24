#include <Arduino.h>

#include "watchdog.h"
#include "debug.h"

// Not sure how I feel about this dependency just to report our IP periodically.
#include "wifi.h"
#include "utils.h"

namespace fk {

constexpr uint32_t Interval = 5000;
constexpr const char Log[] = "Watchdog";

void Watchdog::setup() {
    leds.setup();
}

void Watchdog::tick() {
    if (millis() > time) {
        time = millis() + Interval;
        IpAddress4 ip{ WiFi.localIP() };
        debugfpln(Log, "Tick (%lu free) (%s)", fk_free_memory(), ip.toString());
        leds.alive();
    }
}

}
