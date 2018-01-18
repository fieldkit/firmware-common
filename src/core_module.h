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

namespace fk {

class CoreModule {
private:
    FileSystem fileSystem;
    Watchdog watchdog{ leds };
    Power power{ state };
    CoreState state{fileSystem.getData()};
    Pool modulesPool{"ModulesPool", 256};
    Pool appPool{"AppPool", 256};
    Leds leds;

    HttpTransmissionConfig transmissionConfig = {
        .url = "https://api.fkdev.org/messages/ingestion?token=TOKEN",
    };
    TwoWireBus bus{ Wire };
    HttpPost transmission{wifi, transmissionConfig};
    GatherReadings gatherReadings{bus, state, leds, modulesPool};
    JsonMessageBuilder builder{state, clock};
    SendTransmission sendTransmission{bus, builder, transmission, modulesPool};
    SendStatus sendStatus{bus, builder, transmission, modulesPool};
    DetermineLocation determineLocation{bus, state, modulesPool};
    uint8_t addresses[4]{ 7, 8, 9, 0 };
    AttachedDevices attachedDevices{bus, addresses, state, leds, modulesPool};
    ScheduledTask tasks[4] {
        fk::ScheduledTask{ { -1, 30 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, gatherReadings },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendTransmission },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendStatus },
        fk::ScheduledTask{ { 10, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, determineLocation },
    };
    Scheduler scheduler{state, clock, tasks};

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
