#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include <lwcron/lwcron.h>

#include "core_fsm.h"
#include "rtc.h"
#include "flash_storage.h"
#include "core_state.h"
#include "pool.h"

namespace fk {

class Leds;
class Watchdog;
class TwoWireBus;
class Power;
class Status;
class FileSystem;
class UserButton;
class ModuleCommunications;
class GpsService;

struct MainServices {
    Pool *pool;
    Leds *leds;
    Watchdog *watchdog;
    TwoWireBus *bus;
    Power *power;
    Status *status;
    CoreState *state;
    FlashStorage<PersistedState> *flash;
    FileSystem *fileSystem;
    UserButton *button;
    lwcron::Scheduler *scheduler;
    ModuleCommunications *moduleCommunications;
    GpsService *gps;

    MainServices(Pool *pool, Leds *leds, Watchdog *watchdog, TwoWireBus *bus, Power *power, Status *status, CoreState *state,
                 FlashStorage<PersistedState> *flash, FileSystem *fileSystem, UserButton *button, lwcron::Scheduler *scheduler,
                 ModuleCommunications *moduleCommunications, GpsService *gps) :
        pool(pool), leds(leds), watchdog(watchdog), bus(bus), power(power), status(status), state(state), flash(flash),
        fileSystem(fileSystem), button(button), scheduler(scheduler), moduleCommunications(moduleCommunications), gps(gps) {
    }

    void alive();
    void clear();
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

    WifiServices(Pool *pool, Leds *leds, Watchdog *watchdog, TwoWireBus *bus, Power *power, Status *status, CoreState *state,
                 FlashStorage<PersistedState> *flash, FileSystem *fileSystem, UserButton *button, lwcron::Scheduler *scheduler,
                 ModuleCommunications *moduleCommunications, GpsService *gps,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer,
                 LiveDataManager *liveData) :
        MainServices(pool, leds, watchdog, bus, power, status, state, flash, fileSystem, button, scheduler, moduleCommunications, gps),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer), liveData(liveData) {
    }
};

using WifiServicesState = StateWithContext<WifiServices>;
using MainServicesState = StateWithContext<MainServices>;

class Initialized : public CoreDevice {
public:
    const char *name() const override {
        return "Initialized";
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
