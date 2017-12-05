#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"
#include "live_data.h"

namespace fk {

class AppServicer : public Task {
private:
    AppQueryMessage query;
    MessageBuffer *buffer{ nullptr };
    LiveData *liveData;
    CoreState *state;
    Pool *pool;

public:
    AppServicer(LiveData &liveData, CoreState &state, Pool &pool);

public:
    TaskEval task() override;

public:
    bool handle(MessageBuffer &buffer);
    void handle(AppQueryMessage &query);

};

}

#endif
