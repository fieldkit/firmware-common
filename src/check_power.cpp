#include "check_power.h"
#include "power_management.h"
#include "low_power_sleep.h"
#include "scan_attached_deviecs.h"

namespace fk {

void CheckPower::task() {
    auto percentage = services().power->percentage();
    if (percentage < BatteryLowPowerSleepThreshold) {
        if (!fk_console_attached()) {
            transit<LowPowerSleep>();
            return;
        }
        else {
            log("Console attached, ignoring.");
        }
    }

    if (visited_) {
        back();
        return;
    }

    visited_ = true;

    transit<ScanAttachedDevices>();
}

}
