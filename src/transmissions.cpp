#include "transmissions.h"

namespace fk {

constexpr const char LogName[] = "ModuleData";

using Logger = SimpleLog<LogName>;

ClearModuleData::ClearModuleData() {
}

void ClearModuleData::query(ModuleQueryMessage &message) {
    message.m().type = fk_module_QueryType_QUERY_DATA_CLEAR;
}

void ClearModuleData::reply(ModuleReplyMessage &message) {
    maximumBytes = message.m().data.size;
}

ModuleDataTransfer::ModuleDataTransfer(FileSystem &fileSystem, FileCopySettings settings) : fileSystem(&fileSystem), settings(settings), streamCopier{ buffer.toBufferPtr() } {
}

void ModuleDataTransfer::query(ModuleQueryMessage &message) {
    if (!fileSystem->beginFileCopy(settings)) {
        Logger::error("Failed to begin file copy.");
    }

    auto &fileCopy = fileSystem->files().fileCopy();

    message.m().type = fk_module_QueryType_QUERY_DATA_APPEND;
    message.m().data.size = fileCopy.size();
}

void ModuleDataTransfer::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %d", message.m().data.size);
    maximumBytes = message.m().data.size;
    receivedReply = true;
}

void ModuleDataTransfer::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    ModuleQuery::prepare(message, outgoing);
    /* 
    query(message);

    auto protoWriter = lws::ProtoBufMessageWriter{ outgoing };
    protoWriter.write(fk_module_WireMessageQuery_fields, &message.m());
    */
}

void ModuleDataTransfer::tick(lws::Writer &outgoing) {
    if (receivedReply) {
        Logger::info("Done");
        /*
        auto &fileCopy = fileSystem->files().fileCopy();
        if (!fileCopy.copy(outgoing)) {
            outgoing.close();
        }
        else {
            if (fileCopy.copied() > maximumBytes) {
                outgoing.close();
            }
        }
        */
    }
}

PrepareTransmissionData::PrepareTransmissionData(CoreState &state, FileSystem &fileSystem, ModuleCommunications &communications, FileCopySettings settings) :
    Task("PrepareTransmissionData"), state(&state), moduleDataTransfer(fileSystem, settings), protocol(communications) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(8, clearModuleData);
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
                protocol.push(8, moduleDataTransfer);
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
