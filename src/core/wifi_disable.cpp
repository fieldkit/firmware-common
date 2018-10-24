#include "wifi_disable.h"
#include "idle.h"
#include "wifi.h"
#include "core_state.h"
#include "configuration.h"
#include "no_modules.h"

namespace fk {

void WifiDisable::task() {
    services().wifi->disable();
    services().state->updateIp(0);

    if (!services().state->hasSensorModules()) {
        if (configuration.no_modules_sleep > 0) {
            transit_into<NoModules>();
            return;
        }
    }

    transit<Idle>();
}

}
