#include "tuning.h"
#include "live_data.h"

namespace fk {

LiveData::LiveData(Task &gatherReadings, CoreState &state) :
    ActiveObject("LiveData"), gatherReadings(&gatherReadings), state(&state) {
}

void LiveData::start(uint32_t newInterval) {
    if (interval != newInterval) {
        if (state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
            log("No attached modules.");
            return;
        }

        log("Started");

        interval = newInterval;
        push(*gatherReadings);
    }

    lastRead = millis();
}

void LiveData::stop() {
    interval = 0;
}

void LiveData::done(Task &task) {
    if (interval == 0)  {
        return;
    }

    if (millis() - lastRead > LivePollInactivity) {
        lastRead = 0;
        return;
    }

    if (interval > 0) {
        push(*gatherReadings);
    }
}

void LiveData::error(Task &task) {
}

}
