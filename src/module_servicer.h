#ifndef FK_MODULE_SERVICER_H_INCLUDED
#define FK_MODULE_SERVICER_H_INCLUDED

#define LWS_ENABLE_PROTOBUF
#include <lwstreams/lwstreams.h>

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

class ModuleCallbacks {
public:
    virtual ModuleReadingStatus beginReading(PendingSensorReading &pending) = 0;
    virtual ModuleReadingStatus readingStatus(PendingSensorReading &pending) = 0;
    virtual TaskEval message(ModuleQueryMessage &query, ModuleReplyMessage &reply) {
        return TaskEval::done();
    }

};

class ModuleServicer : public Task {
private:
    TwoWireBus *bus;
    ModuleInfo *info;
    ModuleCallbacks *callbacks;
    PendingSensorReading pending;
    TwoWireMessageBuffer *outgoing;
    TwoWireMessageBuffer *incoming;
    lws::Writer *writer;
    Pool *pool;

public:
    ModuleServicer(TwoWireBus &bus, ModuleInfo &info, ModuleCallbacks &callbacks, TwoWireMessageBuffer &o, TwoWireMessageBuffer &i, lws::Writer &writer, Pool &pool);

public:
    TaskEval task() override;

    void read(size_t bytes);

    TaskEval handle(ModuleQueryMessage &query);

};

}

#endif
