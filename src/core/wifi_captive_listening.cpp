#include "wifi_captive_listening.h"
#include "wifi_listening.h"
#include "configuration.h"

namespace fk {

void WifiCaptiveListening::entry() {
    WifiState::entry();

    began_ = fk_uptime();
    services().watchdog->idling();
}

void WifiCaptiveListening::task() {
    if (fk_uptime() - began_ > configuration.wifi.captivity_time) {
        transit_into<WifiListening>();
        return;
    }

    serve();
}

}
