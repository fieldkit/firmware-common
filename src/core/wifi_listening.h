#ifndef FK_WIFI_LISTENING_H_INCLUDED
#define FK_WIFI_LISTENING_H_INCLUDED

#include "wifi_states.h"
#include "wifi_disable.h"

namespace fk {

class WifiListening : public WifiState {
private:
    uint32_t began_{ 0 };

public:
    const char *name() const override {
        return "WifiListening";
    }

public:
    void entry() override {
        WifiState::entry();
        if (began_ == 0) {
            log("Reset");
            began_ = fk_uptime();
        }
    }

    void task() override {
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

    void react(SchedulerEvent const &se) override {
        if (se.deferred) {
            warn("Scheduler Event!");
            transit(se.deferred);
        }
    }

};

}

#endif
