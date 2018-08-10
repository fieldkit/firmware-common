#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#define LWS_ENABLE_PROTOBUF

#include <lwstreams/lwstreams.h>

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "file_system.h"
#include "file_reader.h"

namespace fk {

class ClearModuleData : public ModuleQuery {
public:
    const char *name() const override {
        return "ClearModuleData";
    }

    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;

public:
    ClearModuleData();

};

class ModuleDataTransfer : public ModuleQuery {
public:
    ModuleDataTransfer();

public:
    const char *name() const override {
        return "ModuleDataTransfer";
    }

    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;

};

class WriteModuleData : public ModuleQuery {
private:
    lws::Reader *reader_;
    lws::BufferedStreamCopier<FileCopyBufferSize> streamCopier_;
    uint32_t started_{ 0 };
    uint32_t total_{ 0 };
    uint32_t copied_{ 0 };
    uint32_t lastStatus_{ 0 };

public:
    WriteModuleData(lws::Reader *reader);

public:
    const char *name() const override {
        return "WriteModuleData";
    }

public:
    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;
    void prepare(ModuleQueryMessage &message, lws::Writer &outgoing) override;
    void tick(lws::Writer &outgoing) override;
    bool replyExpected() override {
        return false;
    }

};

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
    ClearModuleData clearModuleData;
    ModuleDataTransfer moduleDataTransfer;
    WriteModuleData writeModuleData;
    ModuleProtocolHandler protocol;

public:
    PrepareTransmissionData(CoreState &state, ModuleCommunications &communications, lws::Reader *reader);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
