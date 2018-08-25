#ifndef FK_WIFI_CAPTIVE_LISTENING_H_INCLUDED
#define FK_WIFI_CAPTIVE_LISTENING_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiCaptiveListening : public WifiState {
private:
    uint32_t began_{ 0 };

public:
    const char *name() const override {
        return "WifiCaptiveListening";
    }

public:
    void entry() override {
        WifiState::entry();
        began_ = fk_uptime();
    }

    void task() override;

};

}

#endif
