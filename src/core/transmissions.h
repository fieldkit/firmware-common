#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "task.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "file_system.h"
#include "file_reader.h"
#include "checksum_streams.h"
#include "module_comms.h"
#include "module_copy_settings.h"

namespace fk {

class PrepareModuleDataTransfer : public ModuleQuery {
private:
    ModuleCopySettings settings_;

public:
    PrepareModuleDataTransfer(ModuleCopySettings settings);

public:
    const char *name() const override {
        return "PrepareModuleDataTransfer";
    }

public:
    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;
    ReplyConfig replyConfig() override {
        return ReplyConfig::Long;
    }

};

class WriteModuleData : public ModuleQuery {
private:
    lws::SizedReader *reader_;
    lws::BufferedStreamCopier<FileCopyBufferSize> streamCopier_;
    Crc32Reader checksumReader_;
    uint32_t started_{ 0 };
    uint32_t total_{ 0 };
    uint32_t copied_{ 0 };
    uint32_t lastStatus_{ 0 };

public:
    WriteModuleData(lws::SizedReader *reader);

public:
    const char *name() const override {
        return "WriteModuleData";
    }

public:
    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;
    void prepare(ModuleQueryMessage &message, lws::Writer &outgoing) override;
    void tick(lws::Writer &outgoing) override;
    ReplyConfig replyConfig() override {
        return ReplyConfig::NoReply;
    }

public:
    uint32_t checksum() {
        return checksumReader_.checksum();
    }

private:
    void status();

};

class VerifyModuleData : public ModuleQuery {
private:
    ModuleCopySettings settings_;
    uint32_t expectedChecksum_{ 0 };
    pb_data_t checksumData_;

public:
    VerifyModuleData(ModuleCopySettings settings);

public:
    const char *name() const override {
        return "VerifyModuleData";
    }

public:
    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;

public:
    void expectedChecksum(uint32_t value) {
        expectedChecksum_ = value;
    }
};

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
    ModuleProtocolHandler protocol;
    lws::SizedReader *reader;
    PrepareModuleDataTransfer prepareModuleDataTransfer;
    WriteModuleData writeModuleData;
    VerifyModuleData verifyModuleData;
    ModuleCopySettings settings;

public:
    PrepareTransmissionData(CoreState &state, ModuleCommunications &communications, lws::SizedReader *reader, ModuleCopySettings settings);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
