#ifndef FK_POWER_H_INCLUDED
#define FK_POWER_H_INCLUDED

#include <cstdint>
#include <FuelGauge.h>

#include "active_object.h"
#include "core_state.h"

namespace fk {

class Power : public ActiveObject {
private:
    CoreState *state;
    FuelGauge gauge;
    uint32_t time { 0 };

public:
    Power(CoreState &state) : state(&state) {
    }

public:
    void setup();

public:
    void idle() override;

};

}

#endif
