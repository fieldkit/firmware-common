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
    void entry() override;
    void task() override;
    void react(SchedulerEvent const &se) override;

};

}

#endif
