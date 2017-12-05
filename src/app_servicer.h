#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"
#include "live_data.h"
#include "scheduler.h"

namespace fk {

class AppServicer : public Task {
private:
    MessageBuffer *buffer{ nullptr };
    AppQueryMessage query;
    LiveData *liveData;
    CoreState *state;
    Scheduler *scheduler;
    Pool *pool;

public:
    AppServicer(LiveData &liveData, CoreState &state, Scheduler &scheduler, Pool &pool);

public:
    TaskEval task() override;

public:
    bool handle(MessageBuffer &buffer);
    void handle(AppQueryMessage &query);

};

}

#endif
