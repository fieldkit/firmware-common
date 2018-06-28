#include "transmissions.h"

namespace fk {

ClearModuleData::ClearModuleData() {
}

void ClearModuleData::query(ModuleQueryMessage &message) {
    message.m().type = fk_module_QueryType_QUERY_DATA_CLEAR;
}

void ClearModuleData::reply(ModuleReplyMessage &message) {
    maximumBytes = message.m().data.size;
}

ModuleDataTransfer::ModuleDataTransfer(FileSystem &fileSystem, uint8_t file) : fileSystem(&fileSystem), buffer(), streamCopier{ buffer.toBufferPtr() } {
}

void ModuleDataTransfer::query(ModuleQueryMessage &message) {
    if (!fileSystem->openForReading(4)) {
    }

    auto &fileCopy = fileSystem->files().fileCopy();

    message.m().type = fk_module_QueryType_QUERY_DATA_APPEND;
    message.m().data.size = fileCopy.size();
}

void ModuleDataTransfer::reply(ModuleReplyMessage &message) {
    maximumBytes = message.m().data.size;
}

void ModuleDataTransfer::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    query(message);

    auto protoWriter = lws::ProtoBufMessageWriter{ outgoing };
    protoWriter.write(fk_module_WireMessageQuery_fields, &message.m());
}

void ModuleDataTransfer::tick(lws::Writer &outgoing) {
    auto &fileCopy = fileSystem->files().fileCopy();
    if (!fileCopy.tick(outgoing)) {
        outgoing.close();
    }
    else {
        if (fileCopy.copied() > maximumBytes) {
            outgoing.close();
        }
    }
}

PrepareTransmissionData::PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications) :
    Task("PrepareTransmissionData"), state(&state), moduleDataTransfer(fileSystem, file), protocol(communications) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(9, clearModuleData);
}

TaskEval PrepareTransmissionData::task() {
    if (protocol.isBusy()) {
        auto finished = protocol.handle();
        if (finished) {
            if (finished.error()) {
                return TaskEval::error();
            }

            if (finished.is(clearModuleData)) {
                moduleDataTransfer.setMaximumBytes(clearModuleData.getMaximumBytes());
                protocol.push(9, moduleDataTransfer);
            }
            else {
                log("Done");
            }
        }
        return TaskEval::busy();
    }
    return TaskEval::done();
}

}
