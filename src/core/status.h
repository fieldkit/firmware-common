#ifndef FK_STATUS_H_INCLUDED
#define FK_STATUS_H_INCLUDED

#include "task.h"
#include "core_state.h"
#include "leds.h"

namespace fk {

class Status : public Task {
private:
    uint32_t lastLogged{ 0 };
    uint32_t lastTick{ 0 };
    CoreState *state;
    Leds *leds;

public:
    Status(CoreState &state, Leds &leds) : Task("Status"), state(&state), leds(&leds) {
    }

public:
    TaskEval task() override;

};

}

#endif
