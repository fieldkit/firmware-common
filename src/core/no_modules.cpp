#include "no_modules.h"
#include "configuration.h"
#include "reboot_device.h"
#include "scan_attached_devices.h"
#include "low_power_sleep.h"

namespace fk {

void NoModules::react(LowPowerEvent const &lpe) {
    if (fk_console_attached()) {
        log("Console attached, ignoring LowPowerEvent.");
    }
    else {
        transit<LowPowerSleep>();
    }
}

void NoModules::react(SchedulerEvent const &se) {
    if (se.deferred) {
        warn("Scheduler Event!");
        transit(se.deferred);
    }
}

void NoModules::entry() {
    MainServicesState::entry();

    services().watchdog->idling();
}

void NoModules::task() {
    services().alive();

    services().scheduledTasks();

    fk_assert(configuration.no_modules_sleep > 0);

    if (entered_ == 0) {
        entered_ = fk_uptime();
        scanned_ = 0;
    }

    if (configuration.no_modules_rescan > 0) {
        if (scanned_ == 0 || fk_uptime() - scanned_ > configuration.no_modules_rescan) {
            scanned_ = fk_uptime();
            transit<ScanAttachedDevices>();
            return;
        }
    }

    if (fk_uptime() - entered_ > configuration.no_modules_sleep) {
        entered_ = 0;
        scanned_ = 0;
        transit<RebootDevice>();
        return;
    }
}

}
