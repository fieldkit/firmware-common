#ifndef FK_STATUS_H_INCLUDED
#define FK_STATUS_H_INCLUDED

#include "task.h"
#include "core_state.h"
#include "two_wire.h"
#include "leds.h"
#include "utils.h"

namespace fk {

class Status : public Task {
private:
    uint32_t lastLogged{ 0 };
    uint32_t lastTick{ 0 };
    CoreState *state;
    TwoWireBus *bus;
    Leds *leds;

public:
    Status(CoreState &state, TwoWireBus &bus, Leds &leds) : Task("Status"), state(&state), bus(&bus), leds(&leds) {
    }

public:
    TaskEval task() override;

};

}

#endif
