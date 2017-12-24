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
#include "app_servicer.h"
#include "scheduler.h"
#include "rtc.h"
#include "simple_ntp.h"
#include "core.h"
#include "http_post.h"
#include "json_message_builder.h"
#include "discovery.h"
#include "leds.h"

namespace fk {

class CoreModule {
private:
    static constexpr uint8_t FKFS_FILE_LOG = 0;
    static constexpr uint8_t FKFS_FILE_DATA = 1;
    static constexpr uint8_t FKFS_FILE_PRIORITY_LOWEST = 255;
    static constexpr uint8_t FKFS_FILE_PRIORITY_HIGHEST = 0;

    fkfs_t fs = { 0 };
    fkfs_log_t fkfs_log = { 0 };
    Watchdog watchdog;
    FkfsData data{fs, FKFS_FILE_DATA};
    CoreState state{data};
    Clock clock;
    Pool modulesPool{"ModulesPool", 256};
    Pool appPool{"AppPool", 256};
    Leds leds;

    HttpTransmissionConfig transmissionConfig = {
        .url = "https://api.fkdev.org/messages/ingestion?token=TOKEN",
    };
    HttpPost transmission{transmissionConfig};
    GatherReadings gatherReadings{state, modulesPool};
    JsonMessageBuilder builder{state, clock};
    SendTransmission sendTransmission{builder, transmission, modulesPool};
    SendStatus sendStatus{builder, transmission, modulesPool};
    DetermineLocation determineLocation{state, modulesPool};
    ScheduledTask tasks[4] {
        fk::ScheduledTask{ { -1, 30 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, gatherReadings },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendTransmission },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendStatus },
        fk::ScheduledTask{ { 10, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, determineLocation },
    };
    Scheduler scheduler{state, clock, tasks};

    LiveData liveData{state, modulesPool};
    FkfsReplies fileReplies{fs};
    AppServicer appServicer{liveData, state, scheduler, fileReplies, appPool};
    Wifi wifi{state, appServicer};
    Discovery discovery;

public:
    CoreModule();

public:
    void begin();
    CoreState &getState() {
        return state;
    }
    void run();

private:
    bool setupFileSystem();
    bool synchronizeClock();

};

}

#endif
