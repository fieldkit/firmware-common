#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <sd_raw.h>
#include <fkfs.h>
#include <fkfs_log.h>

#include "app_servicer.h"
#include "attached_devices.h"
#include "core_state.h"
#include "debug.h"
#include "i2c.h"
#include "module_controller.h"
#include "pool.h"
#include "wifi.h"
#include "watchdog.h"
#include "power_management.h"
#include "app_servicer.h"
#include "scheduler.h"
#include "rtc.h"
#include "simple_ntp.h"
#include "core.h"
#include "http_post.h"
#include "json_message_builder.h"
#include "discovery.h"
#include "leds.h"
#include "file_system.h"
#include "fkfs_tasks.h"
#include "transmit_readings.h"
#include "gps.h"

namespace fk {

class CoreModule {
private:
    Pool appPool{"AppPool", 256 + 128};
    Pool modulesPool{"ModulesPool", 256 + 128};
    Pool dataPool{"DataPool", 128};

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    Watchdog watchdog{ leds };
    Power power{ state };
    CoreState state{fileSystem.getData()};
    Leds leds;

    HttpTransmissionConfig transmissionConfig = {
        .url = "http://192.168.0.141:8080/messages/ingestion?token=TOKEN",
        .streamUrl = "http://192.168.0.141:8080/messages/ingestion/stream",
    };
    HttpPost transmission{wifi, transmissionConfig};
    GatherReadings gatherReadings{bus, state, leds, modulesPool};
    JsonMessageBuilder builder{state, clock};
    SendTransmission sendTransmission{bus, builder, transmission, modulesPool};
    SendStatus sendStatus{bus, builder, transmission, modulesPool};
    TransmitAllQueuedReadings transmitAllQueuedReadings{fileSystem.fkfs(), 1, state, wifi, transmissionConfig, bus, dataPool};
    ReadGPS readGps{state};
    uint8_t addresses[4]{ 7, 8, 9, 0 };
    AttachedDevices attachedDevices{bus, addresses, state, leds, modulesPool};
    PeriodicTask periodics[2] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 20 * 1000, gatherReadings },
    };
    ScheduledTask scheduled[4] {
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, gatherReadings },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, transmitAllQueuedReadings },
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendStatus },
        fk::ScheduledTask{ { -1, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, readGps },
    };
    Scheduler scheduler{state, clock, scheduled, periodics};

    LiveData liveData{bus, state, leds, modulesPool};
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), appPool};
    Wifi wifi{state, appServicer};
    Discovery discovery{ bus, wifi };

public:
    CoreModule();

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
