#ifndef FK_LOW_POWER_SLEEP_H_INCLUDED
#define FK_LOW_POWER_SLEEP_H_INCLUDED

#include "state_services.h"

namespace fk {

class LowPowerSleep : public MainServicesState {
public:
    const char *name() const override {
        return "LowPowerSleep";
    }

public:
    void task() override;
};

}

#endif
