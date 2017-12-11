#include "core.h"

namespace fk {

GatherReadings::GatherReadings(CoreState &state, Pool &pool) :
    ActiveObject("GatherReadings"), state(&state), beginTakeReading(pool, 8), queryReadingStatus(pool, 8) {
}

void GatherReadings::enqueued() {
    push(beginTakeReading);
    push(delay);
    push(queryReadingStatus);
}

void GatherReadings::done(Task &task) {
    if (areSame(task, queryReadingStatus)) {
        if (queryReadingStatus.isBusy()) {
            push(delay);
            push(queryReadingStatus);
        } else if (queryReadingStatus.isDone()) {
            state->merge(8, queryReadingStatus.replyMessage());
            push(queryReadingStatus);
        }
    }
}

SendTransmission::SendTransmission(CoreState &state, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendTransmission"), state(&state), method(&method) {
}

void SendTransmission::enqueued() {
    push(*method);
}

void SendTransmission::done(Task &task) {
}

SendStatus::SendStatus(CoreState &state, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendStatus"), state(&state), method(&method) {
}

void SendStatus::enqueued() {
    push(*method);
}

void SendStatus::done(Task &task) {
}

DetermineLocation::DetermineLocation(CoreState &state, Pool &pool) :
    ActiveObject("DetermineLocation"), state(&state) {
}

void DetermineLocation::enqueued() {
}

void DetermineLocation::done(Task &task) {
}

}
