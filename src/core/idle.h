#ifndef FK_IDLE_H_INCLUDED
#define FK_IDLE_H_INCLUDED

#include "state_services.h"

namespace fk {

class Idle : public MainServicesState {
private:
    uint32_t checked_{ 0 };
    uint32_t entered_{ 0 };

public:
    const char *name() const override {
        return "Idle";
    }

public:
    void react(LowPowerEvent const &lpe) override;
    void react(SchedulerEvent const &se) override;
    void entry() override;
    void task() override;

private:
    void status(DateTime now, lwcron::Scheduler::TaskAndTime nextTask);
};

}

#endif
