#include "transmissions.h"

namespace fk {

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications, Pool &pool) :
    Task("PrepareTransmissionData"), state(&state), fileSystem(&fileSystem), iterator(fileSystem.fkfs(), file), protocol(communications, pool), pool(&pool) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(queryCaps);
    iterator.beginning();
}

TaskEval PrepareTransmissionData::task() {
    /*
    if (iterator.isFinished()) {
        log("Done");
        return TaskEval::done();
    }

    auto remaining = 128;
    while (remaining > 0 && !iterator.isFinished()) {
        auto data = iterator.read(remaining);
        if (data) {
            remaining -= data.size;
            // log("Block: %p %d", data.ptr, data.size);
        }
    }
    */

    auto step = protocol.handle();
    if (step) {
        if (step.is(queryCaps)) {
            if (step.error()) {
            }

            log("Number: %d", queryCaps.getNumberOfSensors());
            if (queryCaps.getNumberOfSensors() > 0) {
                querySensorCaps = QuerySensorCaps{};
                protocol.push(querySensorCaps);
            }
            else {
                return TaskEval::done();
            }
        }
        else if (step.is(querySensorCaps)) {
            log("Querying sensor caps: %d", querySensorCaps.getSensor());
            if (querySensorCaps.getSensor() < queryCaps.getNumberOfSensors()) {
                protocol.push(querySensorCaps);
            }
            else {
                return TaskEval::done();
            }
        }
        else {
            return TaskEval::error();
        }
    }

    return TaskEval::busy();
}

}
