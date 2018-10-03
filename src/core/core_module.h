#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cinttypes>
#include <cstddef>

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
#include "configuration.h"

#include "begin_gather_readings.h"
#include "wifi_startup.h"
#include "copy_module_data.h"
#include "transmit_lora_data.h"

namespace fk {

class CoreModule {
private:
    ConfigurableStates configurableStates_{
        CoreFsm::deferred<BeginGatherReadings>(),
        CoreFsm::deferred<BeginGatherReadings>()
    };

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
    CoreState state{flashState, fileSystem.logging()};
    ModuleCommunications moduleCommunications{bus, pool};

    CronTask wifiTask{ configuration.schedule.wifi, { CoreFsm::deferred<WifiStartup>() } };
    CronTask readingsTask{ configuration.schedule.readings, { CoreFsm::deferred<BeginGatherReadings>() } };
    CronTask loraTask{ configuration.schedule.lora, { CoreFsm::deferred<TransmitLoraData>() } };

    lwcron::Task *tasks[3] {
        &readingsTask,
        &wifiTask,
        &loraTask
    };

    lwcron::Scheduler scheduler{tasks};

    // Radio stuff.
    #if defined(FK_ENABLE_RADIO)
    RadioService radioService;
    #endif

    // Wifi stuff
    HttpTransmissionConfig httpConfig = {
        .streamUrl = configuration.wifi.stream_url,
    };
    WifiConnection connection;
    AppServicer appServicer{state, fileSystem.replies(), connection, moduleCommunications, pool};
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
        &gps,
        &configurableStates_
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
        &configurableStates_,

        &wifi,
        &discovery,
        &httpConfig,
        &wifi.server(),
        &appServicer,
        &liveData
    };

public:
    CoreModule(ConfigurableStates configurableStates);

public:
    void run();

};

}

#endif
