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
private:
    uint32_t maximumBytes{ 0 };

public:
    const char *name() const override {
        return "ClearModuleData";
    }

    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;

public:
    ClearModuleData();

public:
    uint32_t getMaximumBytes() {
        return maximumBytes;
    }

};

class ModuleDataTransfer : public ModuleQuery {
private:
    FileReader fileReader;
    lws::AlignedStorageBuffer<128> buffer;
    lws::StreamCopier streamCopier;
    uint32_t bytesCopied{ 0 };
    uint32_t maximumBytes{ 0 };

public:
    ModuleDataTransfer(FileSystem &fileSystem, uint8_t file);

public:
    const char *name() const override {
        return "ModuleDataTransfer";
    }

    void query(ModuleQueryMessage &message) override;
    void reply(ModuleReplyMessage &message) override;
    void prepare(ModuleQueryMessage &message, lws::Writer &outgoing) override;
    void tick(lws::Writer &outgoing) override;

public:
    void setMaximumBytes(uint32_t bytes) {
        maximumBytes = bytes;
    }

    uint32_t getMaximumBytes() {
        return maximumBytes;
    }
};

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
    ClearModuleData clearModuleData;
    ModuleDataTransfer moduleDataTransfer;
    ModuleProtocolHandler protocol;

public:
    PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
