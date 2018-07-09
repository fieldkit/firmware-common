#include "watchdog.h"
#include "debug.h"
#include "asf.h"

// Not sure how I feel about this dependency just to report our IP periodically.
#include "wifi.h"
#include "utils.h"
#include "restart_wizard.h"

namespace fk {

void Watchdog::setup() {
    wdt_enable(WDT_PERIOD_8X, false);
}

void Watchdog::started() {
    switch (system_get_reset_cause()) {
    case SYSTEM_RESET_CAUSE_SOFTWARE: log("ResetCause: Software"); break;
    case SYSTEM_RESET_CAUSE_WDT: log("ResetCause: WDT"); break;
    case SYSTEM_RESET_CAUSE_EXTERNAL_RESET: log("ResetCause: External Reset"); break;
    case SYSTEM_RESET_CAUSE_BOD33: log("ResetCause: BOD33"); break;
    case SYSTEM_RESET_CAUSE_BOD12: log("ResetCause: BOD12"); break;
    case SYSTEM_RESET_CAUSE_POR: log("ResetCause: PoR"); break;
    default: {
        log("ResetCause: Unknown");
        break;
    }
    }
}

TaskEval Watchdog::task() {
    fk::restartWizard.looped();

    if (wdt_read_early_warning()) {
        wdt_clear_early_warning();
        wdt_checkin();

        leds->alive();
    }

    return TaskEval::idle();
}

static uint8_t get_period_for_ms(uint16_t ms) {
    if (ms >= 8192) return WDT_PERIOD_8X;
    if (ms >= 4096) return WDT_PERIOD_4X;
    if (ms >= 2048) return WDT_PERIOD_2X;
    if (ms >= 1024) return WDT_PERIOD_1X;
    if (ms >=  512) return WDT_PERIOD_1DIV2;
    return 0;
}

uint32_t Watchdog::sleep(uint32_t ms) {
    auto remaining = ms;

    while (remaining > 0) {
        auto period = get_period_for_ms(remaining);
        if (period > 0) {
            auto time = wdt_enable(WDT_PERIOD_8X, false);
            remaining -= time;
            if (time > 0) {
                system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
                system_sleep();

                fk_uptime_adjust(time);
            }
            else {
                delay(remaining);
                remaining = 0;
            }
        }
        else {
            delay(remaining);
            remaining = 0;
        }
    }

    return ms;
}

}
