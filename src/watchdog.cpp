#include <reset.h>
#undef min
#undef max
#undef HIGH
#undef LOW

#include <AtSamd.h>

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

    switch (system_get_reset_cause()) {
    case SYSTEM_RESET_CAUSE_SOFTWARE: debugfpln(Log, "ResetCause: Software"); break;
    case SYSTEM_RESET_CAUSE_WDT: debugfpln(Log, "ResetCause: WDT"); break;
    case SYSTEM_RESET_CAUSE_EXTERNAL_RESET: debugfpln(Log, "ResetCause: External Reset"); break;
    case SYSTEM_RESET_CAUSE_BOD33: debugfpln(Log, "ResetCause: BOD33"); break;
    case SYSTEM_RESET_CAUSE_BOD12: debugfpln(Log, "ResetCause: BOD12"); break;
    case SYSTEM_RESET_CAUSE_POR: debugfpln(Log, "ResetCause: PoR"); break;
    default: {
        debugfpln(Log, "ResetCause: Unknown");
        break;
    }
    }

    wdt_enable(WDT_PERIOD_8X);
}

void Watchdog::tick() {
    if (millis() > time) {
        time = millis() + Interval;
        IpAddress4 ip{ WiFi.localIP() };
        debugfpln(Log, "Tick (%lu free) (%s)", fk_free_memory(), ip.toString());
    }

    if (wdt_read_early_warning()) {
        leds.alive();
        wdt_checkin();
    }
}

}
