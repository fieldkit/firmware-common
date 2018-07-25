#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "hardware.h"
#include "pool.h"
#include "debug.h"
#include "power_management.h"
#include "watchdog.h"
#include "leds.h"
#include "status.h"
#include "rtc.h"
#include "gps.h"

#include "app_servicer.h"
#include "core_state.h"
#include "wifi.h"
#include "app_servicer.h"
#include "scheduler.h"
#include "discovery.h"
#include "file_system.h"
#include "flash_storage.h"
#include "transmissions.h"
#include "radio_service.h"
#include "user_button.h"
#include "wifi_states.h"
#include "live_data.h"

namespace fk {

class CoreModule {
private:
    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    // Main services.
    Leds leds;
    Watchdog watchdog{ leds };
    Power power{ state };
    UserButton button{ leds };
    Status status{ state, bus, leds };
    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    FlashStorage<PersistedState> flashStorage{ watchdog };
    CoreState state{flashStorage, fileSystem.getData()};
    ModuleCommunications moduleCommunications{bus, modulesPool};
    PrepareTransmissionData prepareTransmissionData{state, fileSystem, moduleCommunications, { FileNumber::Data }};

    // Scheduler stuff.
    // TODO: When these are checked they should be checked in a descending order
    // to avoid starvation.
    #ifdef FK_PROFILE_AMAZON

    CronTask gatherReadingsTask{ lwcron::CronSpec::specific(0, 0), { CoreFsm::deferred<BeginGatherReadings>() } };

    #ifdef FK_WIFI_STARTUP_ONLY
    CronTask wifiStartupTask{ { },                                 { CoreFsm::deferred<WifiStartup>() } };
    #else // FK_WIFI_STARTUP_ONLY
    CronTask wifiStartupTask{ lwcron::CronSpec::specific(0, 10),   { CoreFsm::deferred<WifiStartup>() } };
    #endif // FK_WIFI_STARTUP_ONLY

    lwcron::Task *tasks[2] {
        &gatherReadingsTask,
        &wifiStartupTask
    };

    #else // FK_PROFILE_AMAZON

    PeriodicTask wifiStartupTask{ WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } };
    lwcron::Task *tasks[1] {
        &wifiStartupTask
    };

    #endif // FK_PROFILE_AMAZON
    lwcron::Scheduler scheduler{tasks};

    // Radio stuff.
    #ifdef FK_ENABLE_RADIO
    RadioService radioService;
    SendDataToLoraGateway sendDataToLoraGateway{ radioService, fileSystem, { FileNumber::Data } };
    #endif

    // Wifi stuff
    HttpTransmissionConfig httpConfig = {
        .streamUrl = WifiApiUrlIngestionStream,
    };
    WifiConnection connection;
    AppServicer appServicer{state, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{connection};
    Discovery discovery;

    // GPS stuff
    SerialPort gpsSerial{ Hardware::gpsUart };
    GpsService gps{ state, gpsSerial };

    LiveDataManager liveData;

    // Service collections.
    MainServices mainServices{
        &leds,
        &watchdog,
        &power,
        &status,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        &gps
    };

    WifiServices wifiServices{
        &leds,
        &watchdog,
        &power,
        &status,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        &gps,

        &wifi,
        &discovery,
        &httpConfig,
        &wifi.server(),
        &appServicer,
        &liveData
    };

public:
    void begin();
    void run();

public:
    CoreState &getState() {
        return state;
    }

};

}

#endif
