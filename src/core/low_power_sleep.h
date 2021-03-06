#ifndef FK_LOW_POWER_SLEEP_H_INCLUDED
#define FK_LOW_POWER_SLEEP_H_INCLUDED

#include "state_services.h"
#include "sleep.h"

namespace fk {

class LowPowerSleep : public Sleep {
public:
    const char *name() const override {
        return "LowPowerSleep";
    }

public:
    void entry() override;
    void task() override;
};

}

#endif
