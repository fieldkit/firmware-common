#include "transmissions.h"
#include "performance.h"

namespace fk {

constexpr const char LogName[] = "ModuleData";

using Logger = SimpleLog<LogName>;

PrepareModuleDataTransfer::PrepareModuleDataTransfer(ModuleCopySettings settings) : settings_(settings) {
}

void PrepareModuleDataTransfer::query(ModuleQueryMessage &message) {
    message.m().type = fk_module_QueryType_QUERY_DATA_PREPARE;
    message.m().data.size = settings_.size;
    message.m().data.bank = (uint32_t)settings_.bank;
    message.m().data.etag.arg = (void *)settings_.etag;
}

void PrepareModuleDataTransfer::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %lu", (uint32_t)message.m().data.size);
}

WriteModuleData::WriteModuleData(lws::SizedReader *reader) : reader_(reader), checksumReader_{ *reader } {
}

void WriteModuleData::query(ModuleQueryMessage &message) {
}

void WriteModuleData::reply(ModuleReplyMessage &message) {
}

void WriteModuleData::prepare(ModuleQueryMessage &message, lws::Writer &outgoing) {
    Logger::info("Prepare");
    total_ = reader_->size();
    copied_ = 0;
    started_ = fk_uptime();
    lastStatus_ = started_;
    delay(500);
}

void WriteModuleData::tick(lws::Writer &outgoing) {
    if (total_ > 0) {
        auto bytes = streamCopier_.copy(checksumReader_, outgoing);
        if (bytes == lws::Stream::EOS) {
            status();
            total_ = 0;
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
}

void WriteModuleData::status() {
    auto elapsed = fk_uptime() - started_;
    auto complete = copied_ > 0 ? ((float)copied_ / total_) * 100.0f : 0.0f;
    auto speed = copied_ > 0 ? copied_ / ((float)elapsed / 1000.0f) : 0.0f;
    logtracef("Copy", "%lu/%lu %lums %.2f %.2fbps (%lu)",
              copied_, total_, elapsed, complete, speed, fk_uptime() - lastStatus_);
}

VerifyModuleData::VerifyModuleData(ModuleCopySettings settings) : settings_(settings) {
}

void VerifyModuleData::query(ModuleQueryMessage &message) {
    checksumData_ = {
        .length = sizeof(expectedChecksum_),
        .buffer = &expectedChecksum_,
    };

    message.m().type = fk_module_QueryType_QUERY_DATA_VERIFY;
    message.m().data.size = settings_.size;
    message.m().data.bank = (uint32_t)settings_.bank;
    message.m().data.etag.arg = (void *)settings_.etag;
    message.m().data.checksum.arg = (void *)&checksumData_;
}

void VerifyModuleData::reply(ModuleReplyMessage &message) {
    Logger::info("Reply: %lu", (uint32_t)message.m().data.size);
}

PrepareTransmissionData::PrepareTransmissionData(CoreState &state, ModuleCommunications &communications, lws::SizedReader *reader, ModuleCopySettings settings) :
    Task("PrepareTransmissionData"), state(&state), protocol(communications),
    prepareModuleDataTransfer(settings), writeModuleData(reader), verifyModuleData(settings), settings(settings) {
}

void PrepareTransmissionData::enqueued() {
    protocol.push(8, prepareModuleDataTransfer);
}

TaskEval PrepareTransmissionData::task() {
    if (protocol.isBusy()) {
        auto finished = protocol.handle();
        if (finished) {
            if (finished.error()) {
                return TaskEval::error();
            }

            if (finished.is(prepareModuleDataTransfer)) {
                protocol.push(8, writeModuleData);
            }
            else if (finished.is(writeModuleData)) {
                log("Copy done");
                // TODO: Remove this. Hack to let the module stop waiting for data.
                delay(750);
                verifyModuleData.expectedChecksum(writeModuleData.checksum());
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
