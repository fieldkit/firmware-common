#ifndef FK_CORE_FSM_STATES_H_INCLUDED
#define FK_CORE_FSM_STATES_H_INCLUDED

#include <lwcron/lwcron.h>

#include "core_fsm.h"
#include "state_with_context.h"
#include "rtc.h"
#include "flash_storage.h"
#include "core_state.h"
#include "pool.h"
#include "simple_ntp.h"

namespace fk {

class Leds;
class Watchdog;
class TwoWireBus;
class Power;
class Status;
class SerialFlashFileSystem;
class FileSystem;
class UserButton;
class ModuleCommunications;
class RadioService;
class GpsService;

struct ConfigurableStates {
    tinyfsm::Fsm<fk::CoreDevice>::Deferred configure;
    tinyfsm::Fsm<fk::CoreDevice>::Deferred readings;
};

struct MainServices {
    Pool *pool;
    Leds *leds;
    Watchdog *watchdog;
    TwoWireBus *bus;
    Power *power;
    Status *status;
    CoreState *state;
    SerialFlashFileSystem *flashFs;
    FlashState<PersistedState> *flashState;
    FileSystem *fileSystem;
    UserButton *button;
    lwcron::Scheduler *scheduler;
    ModuleCommunications *moduleCommunications;
    RadioService *radio;
    GpsService *gps;
    ConfigurableStates *states;

    MainServices(Pool *pool, Leds *leds, Watchdog *watchdog, TwoWireBus *bus, Power *power, Status *status, CoreState *state,
                 SerialFlashFileSystem *flashFs, FlashState<PersistedState> *flashState, FileSystem *fileSystem, UserButton *button,
                 lwcron::Scheduler *scheduler,
                 ModuleCommunications *moduleCommunications, RadioService *radio, GpsService *gps, ConfigurableStates *states) :
        pool(pool), leds(leds), watchdog(watchdog), bus(bus), power(power), status(status), state(state), flashFs(flashFs), flashState(flashState),
        fileSystem(fileSystem), button(button), scheduler(scheduler), moduleCommunications(moduleCommunications), radio(radio), gps(gps), states(states) {
    }

    bool alive();
    void clear();
    DateTime scheduledTasks();
};

class Wifi;
class Discovery;
class HttpTransmissionConfig;
class Listen;
class AppServicer;
class LiveDataManager;

struct WifiCheckConfig {
    bool listening{ false };

    WifiCheckConfig() {
    }

    WifiCheckConfig(bool listening) : listening(listening) {
    }
};

struct WifiServices : MainServices {
    Wifi *wifi;
    Discovery *discovery;
    HttpTransmissionConfig *httpConfig;
    Listen *server;
    AppServicer *appServicer;
    LiveDataManager *liveData;
    SimpleNTP ntp;
    WifiCheckConfig config;

    WifiServices(Pool *pool, Leds *leds, Watchdog *watchdog, TwoWireBus *bus, Power *power, Status *status, CoreState *state,
                 SerialFlashFileSystem *flashFs, FlashState<PersistedState> *flashState, FileSystem *fileSystem, UserButton *button,
                 lwcron::Scheduler *scheduler,
                 ModuleCommunications *moduleCommunications, RadioService *radio, GpsService *gps, ConfigurableStates *states,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer,
                 LiveDataManager *liveData) :
        MainServices(pool, leds, watchdog, bus, power, status, state, flashFs, flashState, fileSystem, button, scheduler, moduleCommunications, radio, gps, states),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer), liveData(liveData), ntp(clock) {
    }
};

using MainServicesState = StateWithContext<MainServices, CoreDevice>;
using WifiServicesState = StateWithContext<WifiServices, CoreDevice>;

}

#endif
