#ifndef FK_MODULE_SERVICER_H_INCLUDED
#define FK_MODULE_SERVICER_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "task.h"
#include "module_info.h"
#include "module_messages.h"
#include "module_fsm.h"
#include "module_callbacks.h"

namespace fk {

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
