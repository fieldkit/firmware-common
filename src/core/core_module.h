#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cinttypes>
#include <cstddef>
#include <cstdio>

#include "pool.h"
#include "leds.h"
#include "watchdog.h"
#include "power_management.h"
#include "user_button.h"
#include "status.h"
#include "file_system.h"
#include "flash_storage.h"
#include "core_state.h"
#include "module_comms.h"
#include "scheduler.h"
#include "wifi.h"
#include "discovery.h"
#include "hardware.h"
#include "app_servicer.h"
#include "gps.h"
#include "live_data.h"
#include "radio_service.h"

#include "begin_gather_readings.h"
#include "wifi_startup.h"
#include "copy_module_data.h"
#include "transmit_lora_data.h"

namespace fk {

class CoreModule {
private:
    StaticPool<512> pool{"Main"};

    // Main services.
    Leds leds;
    Watchdog watchdog{ leds };
    Power power{ state };
    UserButton button{ leds };
    Status status{ state, leds };
    TwoWireBus bus{ Wire };
    FileSystem fileSystem;
    SerialFlashFileSystem flashFs{ watchdog };
    FlashState<PersistedState> flashState{ flashFs };
    CoreState state{flashState, fileSystem.getData()};
    ModuleCommunications moduleCommunications{bus, pool};

    #if defined(FK_PROFILE_AMAZON)

    CronTask gatherReadingsTask{ lwcron::CronSpec::specific(0, 0), { CoreFsm::deferred<BeginGatherReadings>() } };

    #if defined(FK_WIFI_STARTUP_ONLY)
    CronTask wifiStartupTask{ { },                                 { CoreFsm::deferred<WifiStartup>() } };
    #else // FK_WIFI_STARTUP_ONLY
    CronTask wifiStartupTask{ lwcron::CronSpec::specific(0, 10),   { CoreFsm::deferred<WifiStartup>() } };
    #endif // FK_WIFI_STARTUP_ONLY

    lwcron::Task *tasks[2] {
        &gatherReadingsTask,
        &wifiStartupTask
    };

    #else // FK_PROFILE_AMAZON

    #if defined(FK_NATURALIST) && defined(FK_ENABLE_RADIO)

    PeriodicTask gatherReadingsTask{ ReadingsInterval, { CoreFsm::deferred<BeginGatherReadings>() } };
    PeriodicTask wifiStartupTask{ WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } };
    PeriodicTask loraTask{ 30, { CoreFsm::deferred<TransmitLoraData>() } };
    lwcron::Task *tasks[3] {
        &gatherReadingsTask,
        &wifiStartupTask,
        &loraTask
    };

    #else

    PeriodicTask gatherReadingsTask{ ReadingsInterval, { CoreFsm::deferred<BeginGatherReadings>() } };
    PeriodicTask wifiStartupTask{ WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } };
    lwcron::Task *tasks[2] {
        &gatherReadingsTask,
        &wifiStartupTask
    };

    #endif

    #endif // FK_PROFILE_AMAZON

    lwcron::Scheduler scheduler{tasks};

    // Radio stuff.
    #if defined(FK_ENABLE_RADIO)
    RadioService radioService;
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
        &flashFs,
        &flashState,
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
        &flashFs,
        &flashState,
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
