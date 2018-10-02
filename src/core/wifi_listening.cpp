#include "wifi_listening.h"

namespace fk {

void WifiListening::entry() {
    WifiState::entry();
    if (began_ == 0) {
        log("Reset");
        began_ = fk_uptime();
    }
}

void WifiListening::task() {
    if (fk_uptime() - began_ > WifiInactivityTimeout) {
        #ifdef FK_WIFI_ALWAYS_ON
        log("FK_WIFI_ALWAYS_ON");
        transit_into<WifiListening>();
        #else
        transit<WifiDisable>();
        #endif
        return;
    }

    serve();
}

void WifiListening::react(SchedulerEvent const &se) {
    if (se.deferred) {
        warn("Scheduler Event!");
        transit(se.deferred);
    }
}

}
