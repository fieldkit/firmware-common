#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include <lwcron/lwcron.h>

#include "core_fsm.h"
#include "rtc.h"

namespace fk {

class Leds;
class Watchdog;
class Power;
class Status;
class CoreState;
class FileSystem;
class UserButton;
class ModuleCommunications;
class GpsService;

struct MainServices {
    Leds *leds;
    Watchdog *watchdog;
    Power *power;
    Status *status;
    CoreState *state;
    FileSystem *fileSystem;
    UserButton *button;
    lwcron::Scheduler *scheduler;
    ModuleCommunications *moduleCommunications;
    GpsService *gps;

    MainServices(Leds *leds, Watchdog *watchdog, Power *power, Status *status, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, lwcron::Scheduler *scheduler, ModuleCommunications *moduleCommunications, GpsService *gps) :
        leds(leds), watchdog(watchdog), power(power), status(status), state(state), fileSystem(fileSystem), button(button),
        scheduler(scheduler), moduleCommunications(moduleCommunications), gps(gps) {
    }

    void alive();
    DateTime scheduledTasks();
};

class Wifi;
class Discovery;
class HttpTransmissionConfig;
class Listen;
class AppServicer;
class LiveDataManager;

struct WifiServices : MainServices {
    Wifi *wifi;
    Discovery *discovery;
    HttpTransmissionConfig *httpConfig;
    Listen *server;
    AppServicer *appServicer;
    LiveDataManager *liveData;

    WifiServices(Leds *leds, Watchdog *watchdog, Power *power, Status *status, CoreState *state, FileSystem *fileSystem,
                 UserButton *button, lwcron::Scheduler *scheduler, ModuleCommunications *moduleCommunications, GpsService *gps,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer,
                 LiveDataManager *liveData) :
        MainServices(leds, watchdog, power, status,state, fileSystem, button, scheduler, moduleCommunications, gps),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer), liveData(liveData) {
    }
};

using WifiServicesState = StateWithContext<WifiServices>;
using MainServicesState = StateWithContext<MainServices>;

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

class CheckPower : public MainServicesState {
private:
    bool visited_{ false };

public:
    const char *name() const override {
        return "CheckPower";
    }

public:
    void task() override;
};

class UserWakeup : public MainServicesState {
public:
    const char *name() const override {
        return "UserWakeup";
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
    const char *name() const override {
        return "BeginGatherReadings";
    }

public:
    void task() override;
};

class IgnoredState : public CoreDevice {
public:
    const char *name() const override {
        return "IgnoredState";
    }

public:
    void entry() override {
        back();
    }
};

}

#endif
