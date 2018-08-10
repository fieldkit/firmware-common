#include "transmissions.h"
#include "performance.h"

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
}

void WriteModuleData::reply(ModuleReplyMessage &message) {
}

void WriteModuleData::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    Logger::info("Prepare");
    total_ = 1024 * 256;
    copied_ = 0;
    started_ = fk_uptime();
    lastStatus_ = started_;
}

void WriteModuleData::tick(lws::Writer &outgoing) {
    auto remaining = total_ - copied_;
    if (remaining > 0) {
        uint8_t data[256];

        auto s = outgoing.write(data, std::min((uint32_t)sizeof(data), remaining));
        if (s > 0) {
            copied_ += s;
        }

        if (fk_uptime() - lastStatus_ > 1000) {
            auto elapsed = fk_uptime() - started_;
            auto complete = copied_ > 0 ? ((float)copied_ / total_) * 100.0f : 0.0f;
            auto speed = copied_ > 0 ? copied_ / ((float)elapsed / 1000.0f) : 0.0f;
            logf(LogLevels::TRACE, "Copy", "%lu/%lu %lums %.2f %.2fbps (%lu)",
                 copied_, total_, elapsed, complete, speed, fk_uptime() - lastStatus_);
            lastStatus_ = fk_uptime();
        }
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
