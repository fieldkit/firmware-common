#include "no_modules.h"
#include "reboot_device.h"
#include "configuration.h"

namespace fk {

void NoModulesThrottle::entry() {
    MainServicesState::entry();
    entered_ = fk_uptime();
}

void NoModulesThrottle::task() {
    services().alive();

    fk_assert(configuration.no_modules_sleep > 0);

    if (fk_uptime() - entered_ > configuration.no_modules_sleep) {
        transit<RebootDevice>();
    }
}

}
