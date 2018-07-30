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

#include "begin_gather_readings.h"
#include "wifi_startup.h"

namespace fk {

class CoreModule {
private:
    StaticPool<512> pool{"Main"};

    // Main services.
    Leds leds;
    Watchdog watchdog{ leds };
    Power power{ state };
    UserButton button{ leds };
    Status status{ state, bus, leds };
    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus };
    FlashStorage<PersistedState> flashStorage{ watchdog };
    CoreState state{flashStorage, fileSystem.getData()};
    ModuleCommunications moduleCommunications{bus, pool};
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

    PeriodicTask gatherReadingsTask{ ReadingsInterval, { CoreFsm::deferred<BeginGatherReadings>() } };
    PeriodicTask wifiStartupTask{ WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } };
    lwcron::Task *tasks[2] {
        &gatherReadingsTask,
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
    AppServicer appServicer{state, fileSystem.getReplies(), connection, moduleCommunications, pool};
    Wifi wifi{connection};
    Discovery discovery;

    // GPS stuff
    SerialPort gpsSerial{ Hardware::gpsUart };
    GpsService gps{ state, gpsSerial };

    LiveDataManager liveData;

    // Service collections.
    MainServices mainServices{
        &pool,
        &leds,
        &watchdog,
        &bus,
        &power,
        &status,
        &state,
        &flashStorage,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        #ifdef FK_ENABLE_RADIO
        &radioService,
        #else
        nullptr,
        #endif
        &gps
    };

    WifiServices wifiServices{
        &pool,
        &leds,
        &watchdog,
        &bus,
        &power,
        &status,
        &state,
        &flashStorage,
        &fileSystem,
        &button,
        &scheduler,
        &moduleCommunications,
        #ifdef FK_ENABLE_RADIO
        &radioService,
        #else
        nullptr,
        #endif
        &gps,

        &wifi,
        &discovery,
        &httpConfig,
        &wifi.server(),
        &appServicer,
        &liveData
    };

public:
    void run(tinyfsm::Fsm<fk::CoreDevice>::Deferred configureState);

};

}

#endif
