#include "gather_readings.h"
#include "leds.h"

namespace fk {

GatherReadings::GatherReadings(TwoWireBus &bus, CoreState &state, Leds &leds, Pool &pool) :
    ActiveObject("GatherReadings"), state(&state), leds(&leds), beginTakeReading(bus, pool, 8), queryReadingStatus(bus, pool, 8) {
}

void GatherReadings::enqueued() {
    if (!peripherals.twoWire1().tryAcquire(this)) {
        log("TwoWire unavailable.");
        return;
    }

    if (state->numberOfModules() == 0) {
        log("No attached modules.");
        return;
    }

    state->takingReading();
    leds->beginReading();
    push(beginTakeReading);
}

void GatherReadings::done(Task &task) {
    if (areSame(task, beginTakeReading)) {
        if (beginTakeReading.getBackoff() > 0) {
            log("Using backoff of %lu", beginTakeReading.getBackoff());
            delay.adjust(beginTakeReading.getBackoff());
        } else {
            delay.adjust(300);
        }
        push(delay);
        push(queryReadingStatus);
    } else if (areSame(task, queryReadingStatus)) {
        if (queryReadingStatus.isBusy()) {
            if (queryReadingStatus.getBackoff() > 0) {
                log("Using backoff of %lu", queryReadingStatus.getBackoff());
                delay.adjust(queryReadingStatus.getBackoff());
            } else {
                delay.adjust(300);
            }
            push(delay);
            push(queryReadingStatus);
        } else if (queryReadingStatus.isDone()) {
            state->merge(8, queryReadingStatus.replyMessage());
            push(queryReadingStatus);
        } else {
            leds->doneReading();
        }
    }

    retries = 0;
}

void GatherReadings::error(Task &task) {
    if (areSame(task, beginTakeReading)) {
        if (retries < NumberOfTwoWireRetries) {
            log("Retry %d/%d", retries, NumberOfTwoWireRetries);
            push(beginTakeReading);
            retries++;
        }
        else {
            retries = 0;
        }
    } else if (areSame(task, queryReadingStatus)) {
        if (retries < NumberOfTwoWireRetries) {
            log("Retry %d/%d", retries, NumberOfTwoWireRetries);
            push(queryReadingStatus);
            retries++;
        }
        else {
            retries = 0;
        }
    }
}

void GatherReadings::error() {
    if (peripherals.twoWire1().isOwner(this)) {
        peripherals.twoWire1().release(this);
    }
    state->doneTakingReading();
}

void GatherReadings::done() {
    if (peripherals.twoWire1().isOwner(this)) {
        peripherals.twoWire1().release(this);
    }
    state->doneTakingReading();
}

}
