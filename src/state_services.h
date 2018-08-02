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
class RadioService;
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
    RadioService *radio;
    GpsService *gps;

    MainServices(Pool *pool, Leds *leds, Watchdog *watchdog, TwoWireBus *bus, Power *power, Status *status, CoreState *state,
                 FlashStorage<PersistedState> *flash, FileSystem *fileSystem, UserButton *button, lwcron::Scheduler *scheduler,
                 ModuleCommunications *moduleCommunications, RadioService *radio, GpsService *gps) :
        pool(pool), leds(leds), watchdog(watchdog), bus(bus), power(power), status(status), state(state), flash(flash),
        fileSystem(fileSystem), button(button), scheduler(scheduler), moduleCommunications(moduleCommunications), radio(radio), gps(gps) {
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
                 ModuleCommunications *moduleCommunications, RadioService *radio, GpsService *gps,
                 Wifi *wifi, Discovery *discovery, HttpTransmissionConfig *httpConfig, Listen *server, AppServicer *appServicer,
                 LiveDataManager *liveData) :
        MainServices(pool, leds, watchdog, bus, power, status, state, flash, fileSystem, button, scheduler, moduleCommunications, radio, gps),
        wifi(wifi), discovery(discovery), httpConfig(httpConfig), server(server), appServicer(appServicer), liveData(liveData) {
    }
};

using WifiServicesState = StateWithContext<WifiServices>;
using MainServicesState = StateWithContext<MainServices>;

}

#endif