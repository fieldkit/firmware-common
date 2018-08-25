#include "check_power.h"
#include "power_management.h"
#include "low_power_sleep.h"
#include "firmware_health_check.h"

namespace fk {

void CheckPower::task() {
    auto percentage = services().power->percentage();
    if (percentage < BatteryLowPowerSleepThreshold) {
        #ifdef FK_ENABLE_LOW_POWER_SLEEP
        if (!fk_console_attached()) {
            transit<LowPowerSleep>();
            return;
        }
        else {
            log("Console attached, ignoring.");
        }
        #else
        log("Low power sleep disabled.");
        #endif
    }

    if (visited_) {
        back();
        return;
    }

    visited_ = true;

    transit<FirmwareHealthCheck>();
}

}
