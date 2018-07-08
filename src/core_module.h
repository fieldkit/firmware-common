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
    FlashStorage<PersistedState> flashStorage;
    CoreState state{flashStorage, fileSystem.getData()};
    ModuleCommunications moduleCommunications{bus, modulesPool};
    PrepareTransmissionData prepareTransmissionData{state, fileSystem, moduleCommunications, { FileNumber::Data }};

    // Scheduler stuff.
    // TODO: When these are checked they should be checked in a descending order
    // to avoid starvation.
    PeriodicTask periodics[3] {
        fk::PeriodicTask{ 1000 * WifiTransmitInterval, { CoreFsm::deferred<WifiStartup>() } },
        fk::PeriodicTask{ 1000 * ReadingsInterval,     { CoreFsm::deferred<BeginGatherReadings>() } },
        // NOTE: Minimum interval is now 5s.
        fk::PeriodicTask{ 1000 * 5,                    { CoreFsm::deferred<CheckPower>() } },
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
    AppServicer appServicer{state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
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
