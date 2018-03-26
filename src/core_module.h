#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <sd_raw.h>
#include <fkfs.h>
#include <fkfs_log.h>

#include "hardware.h"
#include "app_servicer.h"
#include "attached_devices.h"
#include "core_state.h"
#include "debug.h"
#include "two_wire.h"
#include "module_controller.h"
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
#include "fkfs_tasks.h"
#include "transmit_readings.h"
#include "gps.h"
#include "status.h"
#include "flash_storage.h"

namespace fk {

#ifndef FK_API_BASE
#define FK_API_BASE "http://api.fkdev.org"
#endif

constexpr const char API_INGESTION_STREAM[] = FK_API_BASE "/messages/ingestion/stream";

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
    SerialFlashChip serialFlash;
    FlashStorage storage{ serialFlash };
    CoreState state{storage, fileSystem.getData()};
    Power power{ state };

    // Schedule this.
    AttachedDevices attachedDevices{bus, addresses, state, leds, modulesPool};

    HttpTransmissionConfig transmissionConfig = {
        .streamUrl = API_INGESTION_STREAM,
    };
    TransmitAllQueuedReadings transmitAllQueuedReadings{fileSystem.fkfs(), 1, state, wifi, transmissionConfig, bus, dataPool};

    SerialPort gpsPort{ Serial1 };
    ReadGps readGps{state, gpsPort};
    GatherReadings gatherReadings{bus, state, leds, modulesPool};

    PeriodicTask periodics[2] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 60 * 1000, gatherReadings },
    };
    ScheduledTask scheduled[1] {
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, transmitAllQueuedReadings },
    };
    Scheduler scheduler{state, clock, scheduled, periodics};

    LiveData liveData{bus, state, leds, modulesPool};
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), appPool};
    Wifi wifi{state, appServicer};
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
