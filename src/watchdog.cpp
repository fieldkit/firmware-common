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

}
