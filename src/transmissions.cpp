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

ModuleDataTransfer::ModuleDataTransfer(FileSystem &fileSystem, FileCopySettings settings) : fileSystem(&fileSystem), settings(settings) {
}

void ModuleDataTransfer::query(ModuleQueryMessage &message) {
    if (!fileSystem->beginFileCopy(settings)) {
        Logger::error("Failed to begin file copy.");
    }

    auto &fileCopy = fileSystem->files().fileCopy();

    message.m().type = fk_module_QueryType_QUERY_DATA_PREPARE;
    message.m().data.size = fileCopy.size();
}

void ModuleDataTransfer::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %d", message.m().data.size);
    maximumBytes = message.m().data.size;
}

void WriteModuleData::query(ModuleQueryMessage &message) {
    Logger::info("Query");
}

void WriteModuleData::reply(ModuleReplyMessage &message) {
    Logger::info("Reply");
}

void WriteModuleData::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    Logger::info("Prepare");
    maximumBytes = 1024;
}

void WriteModuleData::tick(lws::Writer &outgoing) {
    if (maximumBytes > 0) {
        uint8_t data[128];

        auto s = outgoing.write(data, std::min((uint32_t)sizeof(data), maximumBytes));
        if (s > 0) {
            Logger::trace("Send: %d (%d)", s, maximumBytes);
        }

        maximumBytes -= s;
    }
    else {
        outgoing.close();
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
            else if (finished.is(moduleDataTransfer)) {
                protocol.push(8, writeModuleData);
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
