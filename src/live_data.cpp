#include "live_data.h"

namespace fk {

LiveData::LiveData(CoreState &state, Pool &pool) :
    ActiveObject("LiveData"),
    state(&state), pool(&pool), checkDelay(250), takeReadingsDelay(1000),
    beginTakeReading(pool, 8), queryReadingStatus(pool, 8) {
}

void LiveData::start(uint32_t newInterval) {
    if (interval != newInterval) {
        interval = newInterval;
        takeReadingsDelay = Delay{ interval, true };

        log("Started");

        push(beginTakeReading);
        push(checkDelay);
        push(queryReadingStatus);
    }
}

void LiveData::stop() {
    interval = 0;
}

void LiveData::done(Task &task) {
    if (interval == 0)  {
        return;
    }

    if (areSame(task, queryReadingStatus)) {
        state->merge(8, queryReadingStatus.replyMessage());

        if (queryReadingStatus.isBusy()) {
            push(checkDelay);
            push(queryReadingStatus);
        } else if (queryReadingStatus.isDone()) {
            push(queryReadingStatus);
        }
        else {
            push(takeReadingsDelay);
        }
    }

    if (areSame(task, takeReadingsDelay)) {
        takeReadingsDelay = Delay{ interval, true };
        push(beginTakeReading);
        push(checkDelay);
        push(queryReadingStatus);
    }
}

void LiveData::error(Task &task) {
}

bool LiveData::hasReadings() {
    return false;
}

}
