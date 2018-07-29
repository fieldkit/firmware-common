#ifndef FK_LOW_POWER_SLEEP_H_INCLUDED
#define FK_LOW_POWER_SLEEP_H_INCLUDED

#include "core_fsm_states.h"
#include "power_management.h"
#include "sleep.h"

namespace fk {

class LowPowerSleep : public MainServicesState {
public:
    const char *name() const override {
        return "LowPowerSleep";
    }

public:
    void task() override {
        auto percentage = services().power->percentage();
        if (percentage > BatteryLowPowerResumeThreshold) {
            log("Battery: %f", percentage);
            transit_into<RebootDevice>();
        }
        else {
            transit_into<Sleep>(LowPowerSleepDurationSeconds);
        }
    }
};

}

#endif
