#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include "core_fsm.h"

namespace fk {

class Scheduler;
class AttachedDevices;
class Wifi;

struct MainServices {
    Scheduler *scheduler;
    AttachedDevices *attachedDevices;
    Wifi *wifi;
};

class Idle : public StateWithContext<MainServices> {
private:
    uint32_t checked_{ 0 };

public:
    void entry() override;
    void task() override;

};


}

#endif
