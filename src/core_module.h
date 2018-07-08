#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "hardware.h"
#include "pool.h"
#include "debug.h"
#include "power_management.h"

#include "app_servicer.h"
#include "attached_devices.h"
#include "core_state.h"
#include "two_wire.h"
#include "wifi.h"
#include "watchdog.h"
#include "app_servicer.h"
#include "scheduler.h"
#include "rtc.h"
#include "discovery.h"
#include "leds.h"
#include "file_system.h"
#include "gps.h"
#include "status.h"
#include "flash_storage.h"
#include "transmissions.h"
#include "radio_service.h"
#include "user_button.h"
// #include "core_fsm_states.h"
#include "gather_readings.h"
#include "wifi_states.h"

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
    UserButton button{ leds, fileSystem };
    Status status{ state, bus, leds };
    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    FlashStorage<PersistedState> flashStorage;
    CoreState state{flashStorage, fileSystem.getData()};
    ModuleCommunications moduleCommunications{bus, modulesPool};
    PrepareTransmissionData prepareTransmissionData{bus, state, fileSystem, moduleCommunications, { FileNumber::Data }};

    // Readings stuff.
    SerialPort gpsPort{ Serial1 };
    ReadGps readGps{state, gpsPort};
    GatherReadings gatherReadings{bus, state, leds, moduleCommunications};

    // Scheduler stuff.
    PeriodicTask periodics[2] {
        fk::PeriodicTask{ 1000 * ReadingsInterval,     { CoreFsm::deferred<BeginGatherReadings>() } },
        fk::PeriodicTask{ 1000 * WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } },
    };
    ScheduledTask scheduled[2] {
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { CoreFsm::deferred<BeginGatherReadings>() } },
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { CoreFsm::deferred<WifiStartup>() } },
    };
    Scheduler scheduler{clock, scheduled, periodics};

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
    AppServicer appServicer{bus, state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{connection, appServicer};
    Discovery discovery;

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

        &wifi,
        &discovery,
        &httpConfig,
        &wifi.server(),
        &appServicer
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
