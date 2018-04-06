#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "file_system.h"

namespace fk {

class FileReader : public Reader {
private:
    FkfsStreamingIterator iterator;

public:
    FileReader(FileSystem &fileSystem, uint8_t file);

public:
    int32_t read() override;
    int32_t read(uint8_t *ptr, size_t size) override;
    void close() override;

public:
    void open();

};

class ModuleDataTransfer : public ModuleQuery {
private:
    FileReader fileReader;
    AlignedStorageBuffer<128> buffer;
    StreamCopier streamCopier;

public:
    ModuleDataTransfer(FileSystem &fileSystem, uint8_t file);

public:
    const char *name() const override {
        return "ModuleDataTransfer";
    }

    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_DATA_APPEND;
        message.m().data.size = 0;
    }

    void reply(ModuleReplyMessage &message) override {
    }

    void prepare(ModuleQueryMessage &message, Writer &outgoing) override;
    void tick(Writer &outgoing) override;

};

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
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
