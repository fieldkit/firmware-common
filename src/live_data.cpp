#include "live_data.h"

namespace fk {

LiveData::LiveData(CoreState &state, Pool &pool) :
    ActiveObject("LiveData"),
    state(&state), pool(&pool), delay(1000),
    beginTakeReading(pool, 8), queryReadingStatus(pool, 8) {
}

void LiveData::start(uint32_t newInterval) {
    if (interval != newInterval) {
        interval = newInterval;
        delay = Delay{ interval };

        log("Started");

        push(beginTakeReading);
        push(delay);
        push(queryReadingStatus);
    }
}

void LiveData::stop() {
    interval = 0;
}

void LiveData::done(Task &task) {
    if (areSame(task, queryReadingStatus)) {
        state->merge(8, queryReadingStatus.replyMessage());

        // TODO: This could be better. For example we should probably add a
        // delay if we don't get a value back after an immediate retry.
        if (queryReadingStatus.isDone()) {
            push(queryReadingStatus);
        }
        else {
            if (interval > 0) {
                push(beginTakeReading);
                push(delay);
                push(queryReadingStatus);
            }
        }
    }
}

void LiveData::error(Task &task) {
}

bool LiveData::hasReadings() {
    return false;
}

}
