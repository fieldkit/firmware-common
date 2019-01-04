#ifndef FK_STATUS_H_INCLUDED
#define FK_STATUS_H_INCLUDED

#include "task.h"
#include "core_state.h"
#include "leds.h"

namespace fk {

class Status : public Task {
private:
    uint32_t last_logged_{ 0 };
    uint32_t last_tick_{ 0 };
    CoreState *state_;
    Leds *leds_;

public:
    Status(CoreState &state, Leds &leds) : Task("Status"), state_(&state), leds_(&leds) {
    }

public:
    TaskEval task() override;

};

}

#endif
