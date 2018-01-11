#include "live_data.h"

namespace fk {

LiveData::LiveData(TwoWireBus &bus, CoreState &state, Leds &leds, Pool &pool) :
    ActiveObject("LiveData"),
    interval{ 0 }, state(&state), pool(&pool), gatherReadings(bus, state, leds, pool) {
}

void LiveData::start(uint32_t newInterval) {
    if (interval != newInterval) {
        if (state->numberOfModules() == 0) {
            log("No attached modules.");
            return;
        }

        log("Started");

        interval = newInterval;
        push(gatherReadings);
    }
}

void LiveData::stop() {
    interval = 0;
}

void LiveData::done(Task &task) {
    if (interval == 0)  {
        return;
    }

    if (areSame(task, gatherReadings)) {
        if (interval > 0) {
            push(gatherReadings);
        }
    }
}

void LiveData::error(Task &task) {
}

bool LiveData::hasReadings() {
    return false;
}

}
