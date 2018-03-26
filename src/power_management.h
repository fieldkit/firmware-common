#ifndef FK_POWER_H_INCLUDED
#define FK_POWER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

#include <FuelGauge.h>

namespace fk {

class Power : public Task {
private:
    CoreState *state;
    FuelGauge gauge;
    uint32_t time { 0 };

public:
    Power(CoreState &state) : Task("PM"), state(&state) {
    }

public:
    void setup();

public:
    TaskEval task() override;

};

}

#endif
