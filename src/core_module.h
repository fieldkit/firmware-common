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

namespace fk {

class CoreModule {
private:
    uint8_t addresses[4]{ 7, 8, 9, 0 };

    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    Supervisor<5> background{ true };
    Supervisor<5> servicing{ true };

    Leds leds;
    Watchdog watchdog{ leds };

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    SerialFlashChip serialFlash;
    FlashStorage storage{ serialFlash };
    CoreState state{storage, fileSystem.getData()};
    Power power{ state };

    ModuleCommunications moduleCommunications{bus, background, modulesPool};

    AttachedDevices attachedDevices{bus, addresses, state, leds, moduleCommunications};

    HttpTransmissionConfig transmissionConfig = {
        .streamUrl = WifiApiUrlIngestionStream,
    };
    TransmitFileTask transmitFileTask{fileSystem, state, wifi, transmissionConfig};
    PrepareTransmissionData prepareTransmissionData{bus, state, fileSystem, 0, moduleCommunications};

    FileCopierSample fileCopier{ fileSystem, state };

    SerialPort gpsPort{ Serial1 };
    ReadGps readGps{state, gpsPort};

    GatherReadings gatherReadings{bus, state, leds, moduleCommunications};

    #ifdef FK_ENABLE_RADIO
    RadioService radioService;
    SendDataToLoraGateway sendDataToLoraGateway{ radioService, fileSystem, 0 };
    #endif

    NoopTask noop;

    PeriodicTask periodics[3] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 60 * 1000, gatherReadings },
        fk::PeriodicTask{ 20 * 1000, fileCopier },
    };
    ScheduledTask scheduled[3 ] {
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, prepareTransmissionData },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, transmitFileTask },
        #ifdef FK_ENABLE_RADIO
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendDataToLoraGateway },
        #else
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, noop },
        #endif
    };
    Scheduler scheduler{state, clock, background, scheduled, periodics};

    LiveData liveData{gatherReadings, state};

    WifiConnection connection;
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{state, connection, appServicer, servicing};
    Discovery discovery{ bus, wifi };

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
