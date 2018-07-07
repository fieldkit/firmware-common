#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include "core_fsm.h"

namespace fk {

class Leds;
class Watchdog;
class Power;
class Scheduler;
class AttachedDevices;
class CoreState;
class FileSystem;
class UserButton;

struct MainServices {
    Leds *leds;
    Watchdog *watchdog;
    Power *power;
    CoreState *state;
    FileSystem *fileSystem;
    UserButton *button;
    Scheduler *scheduler;
    AttachedDevices *attachedDevices;

    MainServices(Leds *leds, Watchdog *watchdog, Power *power, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, Scheduler *scheduler, AttachedDevices *attachedDevices) :
        leds(leds), watchdog(watchdog), power(power), state(state), fileSystem(fileSystem), button(button), scheduler(scheduler),
        attachedDevices(attachedDevices) {
    }
};

class Wifi;
class Discovery;
class HttpTransmissionConfig;
class Listen;
class AppServicer;

struct WifiServices : MainServices {
    Wifi *wifi;
    Discovery *discovery;
    HttpTransmissionConfig *httpConfig;
    Listen *server;
    AppServicer *appServicer;

    WifiServices(Leds *leds, Watchdog *watchdog, Power *power, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, Scheduler *scheduler, AttachedDevices *attachedDevices,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer) :
        MainServices(leds, watchdog, power, state, fileSystem, button, scheduler, attachedDevices),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer) {
    }
};

using MainServicesState = StateWithContext<MainServices>;
using WifiServicesState = StateWithContext<WifiServices>;

class Idle : public MainServicesState {
private:
    uint32_t checked_{ 0 };
    uint32_t began_{ 0 };

public:
    void entry() override;
    void task() override;

};

class BeginGatherReadings : public MainServicesState {
public:
    void entry() override;
    void task() override;

};

class IgnoredState : public CoreDevice {
public:
    void entry() override {
        back();
    }

};

}

#endif
