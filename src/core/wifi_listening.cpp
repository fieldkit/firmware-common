#include "wifi_listening.h"
#include "configuration.h"

namespace fk {

void WifiListening::entry() {
    WifiState::entry();
    if (began_ == 0) {
        log("Reset");
        began_ = fk_uptime();
    }
}

void WifiListening::task() {
    if (configuration.wifi.inactivity_time > 0) {
        if (fk_uptime() - began_ > configuration.wifi.inactivity_time) {
            transit<WifiDisable>();
            return;
        }
    }

    serve();
}

void WifiListening::react(SchedulerEvent const &se) {
    if (se.deferred) {
        transit(se.deferred);
    }
}

}
