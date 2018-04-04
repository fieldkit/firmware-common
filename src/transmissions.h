#ifndef FK_TRANSMISSIONS_H_INCLUDED
#define FK_TRANSMISSIONS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "file_system.h"

namespace fk {

class PrepareTransmissionData : public Task {
private:
    CoreState *state;
    FileSystem *fileSystem;
    FkfsStreamingIterator iterator;
    ModuleCommunications *communications;
    Pool *pool;
    size_t counter{ 3 };

public:
    PrepareTransmissionData(TwoWireBus &bus, CoreState &state, FileSystem &fileSystem, uint8_t file, ModuleCommunications &communications, Pool &pool);

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
