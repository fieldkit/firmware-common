#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "hardware.h"
#include "app_servicer.h"
#include "attached_devices.h"
#include "core_state.h"
#include "debug.h"
#include "two_wire.h"
#include "pool.h"
#include "wifi.h"
#include "watchdog.h"
#include "power_management.h"
#include "app_servicer.h"
#include "scheduler.h"
#include "rtc.h"
#include "simple_ntp.h"
#include "discovery.h"
#include "leds.h"
#include "file_system.h"
#include "transmit_file.h"
#include "gps.h"
#include "status.h"
#include "flash_storage.h"
#include "transmissions.h"
#include "radio_service.h"
#include "user_button.h"
#include "core_fsm_states.h"

namespace fk {

class CoreModule {
private:
    uint8_t addresses[4]{ 7, 8, 9, 0 };

    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    Leds leds;
    Watchdog watchdog{ leds };

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    FlashStorage<PersistedState> flashStorage;
    CoreState state{flashStorage, fileSystem.getData()};
    Power power{ state };

    ModuleCommunications moduleCommunications{bus, modulesPool};

    AttachedDevices attachedDevices{bus, addresses, state, leds, moduleCommunications};

    HttpTransmissionConfig httpConfig = {
        .streamUrl = WifiApiUrlIngestionStream,
    };
    PrepareTransmissionData prepareTransmissionData{bus, state, fileSystem, moduleCommunications, { FileNumber::Data }};

    SerialPort gpsPort{ Serial1 };
    ReadGps readGps{state, gpsPort};

    GatherReadings gatherReadings{bus, state, leds, moduleCommunications};

    #ifdef FK_ENABLE_RADIO
    RadioService radioService;
    SendDataToLoraGateway sendDataToLoraGateway{ radioService, fileSystem, { FileNumber::Data } };
    #endif

    #ifdef FK_PROFILE_AMAZON
    PeriodicTask periodics[1] {
    fk::PeriodicTask{ 60 * 1000, { CoreFsm::deferred<IgnoredState>() } },
    };
    #else
    PeriodicTask periodics[2] {
    fk::PeriodicTask{ 20 * 1000, { CoreFsm::deferred<IgnoredState>() } },
    fk::PeriodicTask{ 60 * 1000, { CoreFsm::deferred<IgnoredState>() } },
    };
    #endif
    ScheduledTask scheduled[1] {
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, { CoreFsm::deferred<BeginGatherReadings>() } },
    };
    Scheduler scheduler{clock, scheduled, periodics};

    LiveData liveData{gatherReadings, state};

    WifiConnection connection;
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{connection, appServicer};
    Discovery discovery;
    UserButton button{ leds, fileSystem };
    Status status{ state, bus, leds };

    MainServices mainServices{
        &leds,
        &watchdog,
        &power,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &attachedDevices,
    };

    WifiServices wifiServices{
        &leds,
        &watchdog,
        &power,
        &state,
        &fileSystem,
        &button,
        &scheduler,
        &attachedDevices,

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
