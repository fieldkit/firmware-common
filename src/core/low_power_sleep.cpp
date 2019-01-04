#include "low_power_sleep.h"
#include "hardware.h"
#include "power_management.h"
#include "sleep.h"
#include "reboot_device.h"

namespace fk {

void LowPowerSleep::entry() {
    Sleep::entry();

    log("Sleeping, disabling modules and peripherals.");

    services().fileSystem->flush();

    Hardware::disableModules();

    Hardware::disablePeripherals();

    services().watchdog->idling();
}

void LowPowerSleep::task() {
    auto status = services().power->status();
    auto percentage = status.percentage;
    if (percentage > BatteryLowPowerResumeThreshold) {
        trace("Battery: %f", percentage);
        transit_into<RebootDevice>();
        return;
    }

    sleep(LowPowerSleepDurationSeconds);

    transit_into<LowPowerSleep>();
}

}
