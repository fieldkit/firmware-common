#include "core.h"
#include "leds.h"

namespace fk {

GatherReadings::GatherReadings(TwoWireBus &bus, CoreState &state, Leds &leds, Pool &pool) :
    ActiveObject("GatherReadings"), state(&state), leds(&leds), beginTakeReading(bus, pool, 8), queryReadingStatus(bus, pool, 8) {
}

void GatherReadings::enqueued() {
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
}

SendTransmission::SendTransmission(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendTransmission"), builder(&builder), method(&method) {
}

void SendTransmission::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendTransmission::done(Task &task) {
}

SendStatus::SendStatus(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendStatus"), builder(&builder), method(&method) {
}

void SendStatus::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendStatus::done(Task &task) {
}

}
