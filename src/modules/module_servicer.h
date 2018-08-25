#ifndef FK_MODULE_SERVICER_H_INCLUDED
#define FK_MODULE_SERVICER_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "task.h"
#include "module_info.h"
#include "module_messages.h"
#include "module_fsm.h"

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

public:
    virtual DeferredModuleState beginReadingState() = 0;

};

class ModuleServicer : public ModuleServicesState {
private:
    PendingSensorReading pending;

public:
    const char *name() const override {
        return "ModuleServicer";
    }

public:
    void task() override;

private:
    void handle(ModuleQueryMessage &query);

};

}

#endif
