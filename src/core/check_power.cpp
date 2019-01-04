#include "check_power.h"
#include "power_management.h"
#include "low_power_sleep.h"
#include "firmware_health_check.h"
#include "configuration.h"

namespace fk {

void CheckPower::task() {
    services().alive();

    auto battery = services().power->status();
    if (battery.low) {
        if (configuration.sleeping.low_power) {
            if (!battery.attached) {
                log("No battery attached.");
            }
            else {
                if (!fk_console_attached()) {
                    log("Battery: %fmv / %f (%fmAh %fma)", battery.voltage, battery.percentage, battery.coulombs, battery.ma);
                    transit<LowPowerSleep>();
                    return;
                }
                else {
                    log("Console attached, ignoring.");
                }
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
