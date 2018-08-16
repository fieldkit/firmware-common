#include "watchdog.h"
#include "platform.h"
#include "tuning.h"

namespace fk {

void Watchdog::setup() {
    fk_wdt_enable();
}

void Watchdog::started() {
    log("ResetCause: %s", fk_system_get_reset_cause());
}

TaskEval Watchdog::task() {
    if (fk_wdt_early_warning_read()) {
        fk_wdt_early_warning_clear();
        fk_wdt_checkin();

        leds_->alive();
    }

    if (elapsedSinceIdle() > MaximumWaitBeforeReturnToIdle) {
        error("No idle for %lu", elapsedSinceIdle());
        fk_system_reset();
    }

    return TaskEval::idle();
}

void Watchdog::idling() {
    idledAt_ = fk_uptime();
}

uint32_t Watchdog::sleep(uint32_t ms) {
    auto remaining = ms;

    while (remaining > 0) {
        auto period = fk_wdt_period(remaining);
        if (period > 0) {
            auto time = fk_wdt_enable();
            remaining -= time;
            if (time > 0) {
                // Adjust before, because IRQs will fire immediately and see
                // the old time.
                fk_uptime_adjust(time);
                fk_system_sleep();
            }
            else {
                fk_delay(remaining);
                remaining = 0;
            }
        }
        else {
            auto end = fk_uptime() + remaining;
            while (fk_uptime() < end) {
                fk_delay(100);
                task();
            }
            remaining = 0;
        }
    }

    return ms;
}

}
