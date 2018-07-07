#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include "core_fsm.h"

namespace fk {

class Scheduler;
class AttachedDevices;
class CoreState;
class Watchdog;

struct MainServices {
    Scheduler *scheduler;
    AttachedDevices *attachedDevices;
    CoreState *state;
    Watchdog *watchdog;

    MainServices(Scheduler *scheduler, AttachedDevices *attachedDevices, CoreState *state, Watchdog *watchdog) :
        scheduler(scheduler), attachedDevices(attachedDevices), state(state), watchdog(watchdog) {
    }
};

class Wifi;
class Listen;
class Discovery;

struct WifiServices : MainServices {
    Wifi *wifi;
    Discovery *discovery;
    Listen *server;

    WifiServices(Scheduler *scheduler, AttachedDevices *attachedDevices, CoreState *state, Watchdog *watchdog,
                 Wifi *wifi, Discovery *discovery, Listen *server) :
        MainServices(scheduler, attachedDevices, state, watchdog),
        wifi(wifi), discovery(discovery), server(server) {
    }
};

using MainServicesState = StateWithContext<MainServices>;
using WifiServicesState = StateWithContext<WifiServices>;

class Idle : public MainServicesState {
private:
    uint32_t checked_{ 0 };

public:
    void entry() override;
    void task() override;

};


}

#endif
