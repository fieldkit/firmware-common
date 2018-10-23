#include "no_modules.h"
#include "configuration.h"
#include "reboot_device.h"
#include "scan_attached_devices.h"

namespace fk {

void NoModulesThrottle::task() {
    services().alive();

    fk_assert(configuration.no_modules_sleep > 0);

    if (entered_ == 0) {
        entered_ = fk_uptime();
        scanned_at_ = 0;
    }

    if (configuration.no_modules_rescan > 0) {
        if (scanned_at_ == 0 || fk_uptime() - scanned_at_ > configuration.no_modules_rescan) {
            scanned_at_ = fk_uptime();
            transit<ScanAttachedDevices>();
            return;
        }
    }

    if (fk_uptime() - entered_ > configuration.no_modules_sleep) {
        transit<RebootDevice>();
        return;
    }
}

}
