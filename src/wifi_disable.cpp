#include "wifi_disable.h"
#include "idle.h"
#include "core_state.h"
#include "wifi.h"

namespace fk {

void WifiDisable::task() {
    services().wifi->disable();
    services().state->updateIp(0);
    transit<Idle>();
}

}
