#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include "core_fsm.h"

namespace fk {

class Leds;
class Watchdog;
class Power;
class Status;
class Scheduler;
class ModuleCommunications;
class CoreState;
class FileSystem;
class UserButton;

struct MainServices {
    Leds *leds;
    Watchdog *watchdog;
    Power *power;
    Status *status;
    CoreState *state;
    FileSystem *fileSystem;
    UserButton *button;
    Scheduler *scheduler;
    ModuleCommunications *moduleCommunications;

    MainServices(Leds *leds, Watchdog *watchdog, Power *power, Status *status, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, Scheduler *scheduler, ModuleCommunications *moduleCommunications) :
        leds(leds), watchdog(watchdog), power(power), status(status), state(state), fileSystem(fileSystem), button(button),
        scheduler(scheduler), moduleCommunications(moduleCommunications) {
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

    WifiServices(Leds *leds, Watchdog *watchdog, Power *power, Status *status, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, Scheduler *scheduler, ModuleCommunications *moduleCommunications,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer) :
        MainServices(leds, watchdog, power, status,state, fileSystem, button, scheduler, moduleCommunications),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer) {
    }
};

using WifiServicesState = StateWithContext<WifiServices>;

class MainServicesState : public StateWithContext<MainServices> {
public:
    void alive();

};

class Idle : public MainServicesState {
private:
    uint32_t checked_{ 0 };
    uint32_t began_{ 0 };

public:
    const char *name() const override {
        return "Idle";
    }

public:
    void react(SchedulerEvent const &se) override;
    void entry() override;
    void task() override;

};

class CheckPower : public MainServicesState {
public:
    const char *name() const override {
        return "CheckPower";
    }

public:
    void task() override;
};

class RebootDevice : public MainServicesState {
public:
    const char *name() const override {
        return "RebootDevice";
    }

public:
    void task() override;

};

class BeginGatherReadings : public MainServicesState {
public:
    void task() override;

public:
    const char *name() const override {
        return "BeginGatherReadings";
    }

};

class IgnoredState : public CoreDevice {
public:
    void entry() override {
        back();
    }

public:
    const char *name() const override {
        return "IgnoredState";
    }

};

}

#endif
