#include "check_power.h"
#include "power_management.h"
#include "low_power_sleep.h"
#include "firmware_health_check.h"
#include "configuration.h"

namespace fk {

void CheckPower::task() {
    auto percentage = services().power->percentage();
    if (percentage < BatteryLowPowerSleepThreshold) {
        if (configuration.sleeping.low_power) {
            if (!fk_console_attached()) {
                transit<LowPowerSleep>();
                return;
            }
            else {
                log("Console attached, ignoring.");
            }
        }
        else {
            log("Low power sleep disabled in configuration.");
        }
    }

    if (visited_) {
        back();
        return;
    }

    visited_ = true;

    transit<FirmwareHealthCheck>();
}

}
