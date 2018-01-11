#ifndef FK_MODULE_SERVICER_H_INCLUDED
#define FK_MODULE_SERVICER_H_INCLUDED

#include "active_object.h"
#include "message_buffer.h"
#include "module_info.h"
#include "module_messages.h"

namespace fk {

struct ModuleReadingStatus {
    uint32_t backoff{ 0 };

    ModuleReadingStatus() {
    }

    ModuleReadingStatus(uint32_t backoff) : backoff(backoff) {
    }
};

// TODO: Rename this
class ModuleCallbacks {
public:
    virtual ModuleReadingStatus beginReading(PendingSensorReading &pending) = 0;
    virtual ModuleReadingStatus readingStatus(PendingSensorReading &pending) = 0;

};

class ModuleServicer : public Task {
private:
    TwoWireBus *bus;
    ModuleInfo *info;
    ModuleCallbacks *callbacks;
    PendingSensorReading pending;
    TwoWireMessageBuffer *outgoing;
    TwoWireMessageBuffer *incoming;
    Pool *pool;

public:
    ModuleServicer(TwoWireBus &bus, ModuleInfo &info, ModuleCallbacks &callbacks, TwoWireMessageBuffer &o, TwoWireMessageBuffer &i, Pool &pool);

public:
    TaskEval task() override;

    void read(size_t bytes);
    bool handle(ModuleQueryMessage &query);

};

}

#endif
