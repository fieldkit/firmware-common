#ifndef FK_POWER_H_INCLUDED
#define FK_POWER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

#include <FuelGauge.h>

namespace fk {

class Power : public Task {
private:
    CoreState *state_;
    FuelGauge gauge_;
    uint32_t queryTime_{ 0 };
    uint32_t lastAlert_{ 0 };

public:
    Power(CoreState &state) : Task("PM"), state_(&state) {
    }

public:
    void setup();
    float percentage();
    TaskEval task() override;

};

}

#endif
