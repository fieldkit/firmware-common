#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "module_controller.h"
#include "core_state.h"

namespace fk {

class AppServicer : public Task {
private:
    AppQueryMessage query;
    MessageBuffer outgoing;
    ModuleController *modules;
    CoreState *state;
    Pool *pool;

public:
    AppServicer(ModuleController &modules, CoreState &state, Pool &pool);

public:
    TaskEval task() override;

    MessageBuffer &outgoingBuffer() {
        return outgoing;
    }
    bool read(MessageBuffer &buffer);
    void handle(AppQueryMessage &query);

};

}

#endif
