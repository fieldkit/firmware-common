#ifndef FK_IDLE_H_INCLUDED
#define FK_IDLE_H_INCLUDED

#include "core_fsm_states.h"

namespace fk {

class Idle : public MainServicesState {
private:
    uint32_t checked_{ 0 };

public:
    const char *name() const override {
        return "Idle";
    }

public:
    void react(LowPowerEvent const &lpe) override;
    void react(SchedulerEvent const &se) override;
    void entry() override;
    void task() override;
};

}

#endif
