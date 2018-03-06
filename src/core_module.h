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
#include "http_post.h"
#include "json_message_builder.h"
#include "discovery.h"
#include "leds.h"
#include "file_system.h"
#include "fkfs_tasks.h"
#include "transmit_readings.h"
#include "gps.h"
#include "status.h"

namespace fk {

#ifndef FK_API_BASE
#define FK_API_BASE "http://api.fkdev.org"
#endif

constexpr const char API_INGESTION_JSON[] = FK_API_BASE "/messages/ingestion?token=TOKEN";
constexpr const char API_INGESTION_STREAM[] = FK_API_BASE "/messages/ingestion/stream";

class CoreModule {
private:
    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    Watchdog watchdog{ leds };
    Power power{ state };
    CoreState state{fileSystem.getData()};
    Leds leds;

    HttpTransmissionConfig transmissionConfig = {
        .url = API_INGESTION_JSON,
        .streamUrl = API_INGESTION_STREAM,
    };
    HttpPost transmission{wifi, transmissionConfig};
    JsonMessageBuilder builder{state, clock};
    SendTransmission sendTransmission{bus, builder, transmission, modulesPool};
    SendStatus sendStatus{bus, builder, transmission, modulesPool};
    TransmitAllQueuedReadings transmitAllQueuedReadings{fileSystem.fkfs(), 1, state, wifi, transmissionConfig, bus, dataPool};
    uint8_t addresses[4]{ 7, 8, 9, 0 };
    AttachedDevices attachedDevices{bus, addresses, state, leds, modulesPool};

    ReadGPS readGps{state, Hardware::gpsUart};
    GatherReadings gatherReadings{bus, state, leds, modulesPool};
    PeriodicTask periodics[2] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 60 * 1000, gatherReadings },
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
    void begin();
    void run();

public:
    CoreState &getState() {
        return state;
    }

};

}

#endif
