#include "low_power_sleep.h"
#include "power_management.h"
#include "sleep.h"
#include "reboot_device.h"

namespace fk {

void LowPowerSleep::entry() {
    MainServicesState::entry();

    services().watchdog->idling();
}

void LowPowerSleep::task() {
    auto percentage = services().power->percentage();
    if (percentage > BatteryLowPowerResumeThreshold) {
        log("Battery: %f", percentage);
        transit_into<RebootDevice>();
    }
    else {
        transit_into<Sleep>(LowPowerSleepDurationSeconds);
    }
}

}
