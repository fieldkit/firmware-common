#include "core.h"

namespace fk {

GatherReadings::GatherReadings(CoreState &state, Pool &pool) :
    Task("GatherReadings"), state(&state), beginTakeReading(pool, 8), queryReadingStatus(pool, 8) {
}

TaskEval GatherReadings::task() {
    return TaskEval::yield(beginTakeReading);
}

SendTransmission::SendTransmission(CoreState &state, Pool &pool) :
    Task("SendTransmission"), state(&state) {
}

TaskEval SendTransmission::task() {
    return TaskEval::done();
}

SendStatus::SendStatus(CoreState &state, Pool &pool) :
    Task("SendStatus"), state(&state) {
}

TaskEval SendStatus::task() {
    return TaskEval::done();
}

DetermineLocation::DetermineLocation(CoreState &state, Pool &pool) :
    Task("DetermineLocation"), state(&state) {
}

TaskEval DetermineLocation::task() {
    return TaskEval::done();
}

Core::Core() : ActiveObject("Core") {
}

}
