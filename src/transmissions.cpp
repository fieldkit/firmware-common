#include "transmissions.h"

namespace fk {

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications) :
    Task("PrepareTransmissionData"), state(&state), fileSystem(&fileSystem), iterator(fileSystem.fkfs(), file), protocol(communications) {
}

void PrepareTransmissionData::enqueued() {
    // protocol.push(queryCaps);
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
    return TaskEval::done();
}

}
