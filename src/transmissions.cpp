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
}

ModuleDataTransfer::ModuleDataTransfer() {
}

void ModuleDataTransfer::query(ModuleQueryMessage &message) {
    message.m().type = fk_module_QueryType_QUERY_DATA_PREPARE;
    message.m().data.size = 0;
}

void ModuleDataTransfer::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %lu", (uint32_t)message.m().data.size);
}

WriteModuleData::WriteModuleData(lws::Reader *reader) : reader_(reader) {
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
        auto bytes = streamCopier_.copy(*reader_, outgoing);
        if (bytes == lws::Stream::EOS) {
            outgoing.close();
        }
        else if (bytes > 0) {
            copied_ += bytes;
        }

        if (fk_uptime() - lastStatus_ > FileCopyStatusInterval) {
            status();
            lastStatus_ = fk_uptime();
        }
    }
    else {
        outgoing.close();
    }
}

void WriteModuleData::status() {
    auto elapsed = fk_uptime() - started_;
    auto complete = copied_ > 0 ? ((float)copied_ / total_) * 100.0f : 0.0f;
    auto speed = copied_ > 0 ? copied_ / ((float)elapsed / 1000.0f) : 0.0f;
    logtracef("Copy", "%lu/%lu %lums %.2f %.2fbps (%lu)",
              copied_, total_, elapsed, complete, speed, fk_uptime() - lastStatus_);
}

VerifyModuleData::VerifyModuleData() {
}

void VerifyModuleData::query(ModuleQueryMessage &message) {
    checksumData = {
        .length = sizeof(expectedChecksum_),
        .buffer = &expectedChecksum_,
    };

    message.m().type = fk_module_QueryType_QUERY_DATA_VERIFY;
    message.m().data.checksum.arg = (void *)&checksumData;
}

void VerifyModuleData::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %lu", (uint32_t)message.m().data.size);
}

PrepareTransmissionData::PrepareTransmissionData(CoreState &state, ModuleCommunications &communications, lws::Reader *reader) :
    Task("PrepareTransmissionData"), state(&state), protocol(communications), checksumReader(*reader), moduleDataTransfer(), writeModuleData(&checksumReader) {
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
                protocol.push(8, moduleDataTransfer);
            }
            else if (finished.is(moduleDataTransfer)) {
                protocol.push(8, writeModuleData);
            }
            else if (finished.is(writeModuleData)) {
                log("Copy done (checksum = 0x%lx)", checksumReader.checksum());

                // TODO: Remove this. Hack to let the module stop waiting for data.
                delay(500);
                verifyModuleData.expectedChecksum(checksumReader.checksum());
                protocol.push(8, verifyModuleData);
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
