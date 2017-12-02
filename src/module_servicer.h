#ifndef FK_MODULE_SERVICER_H_INCLUDED
#define FK_MODULE_SERVICER_H_INCLUDED

#include "active_object.h"
#include "module_info.h"
#include "message_buffer.h"
#include "module_messages.h"

namespace fk {

// TODO: Rename this
class ModuleCallbacks {
public:
    virtual void beginReading(SensorReading *readings) = 0;

};

class ModuleServicer : public Task {
private:
    ModuleInfo *info;
    ModuleCallbacks *callbacks;
    MessageBuffer &outgoing;
    MessageBuffer &incoming;
    Pool *pool;

public:
    ModuleServicer(ModuleInfo *info, ModuleCallbacks &callbacks, MessageBuffer &o, MessageBuffer &i, Pool &pool);

public:
    void read(size_t bytes);
    TaskEval &task() override;

};

}

#endif
