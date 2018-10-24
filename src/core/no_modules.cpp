#include "no_modules.h"
#include "configuration.h"
#include "reboot_device.h"
#include "scan_attached_devices.h"

namespace fk {

void NoModules::task() {
    services().alive();

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
        transit<RebootDevice>();
        return;
    }
}

}
