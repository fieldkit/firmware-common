#include "transmissions.h"

namespace fk {

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications, Pool &pool) :
    Task("PrepareTransmissionData"), state(&state), fileSystem(&fileSystem), iterator(fileSystem.fkfs(), file), communications(&communications), pool(&pool) {
}

void PrepareTransmissionData::enqueued() {
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

    if (!communications->busy()) {
        if (counter == 0) {
            counter = 3;
            return TaskEval::done();
        }

        ModuleQueryMessage query{ *pool };
        query.m().type = fk_module_QueryType_QUERY_DATA_CLEAR;
        communications->enqueue(9, query);

        counter--;
    }
    else {
        if (communications->available()) {
            auto reply = communications->dequeue();
            log("Reply: %d", reply.m().type);
        }
    }

    return TaskEval::busy();
}

}
