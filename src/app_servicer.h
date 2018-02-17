#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"
#include "live_data.h"
#include "scheduler.h"
#include "fkfs_replies.h"

#include "task_container.h"
#include "app_module_query_task.h"

namespace fk {

class AppServicer : public Task {
private:
    TwoWireBus *bus;
    MessageBuffer *buffer{nullptr};
    AppQueryMessage query;
    AppReplyMessage reply;
    LiveData *liveData;
    CoreState *state;
    Scheduler *scheduler;
    FkfsReplies *fileReplies;
    TaskContainer<AppModuleQueryTask> appModuleQueryTask;
    Pool *pool;

public:
    AppServicer(TwoWireBus &bus, LiveData &liveData, CoreState &state, Scheduler &scheduler, FkfsReplies &fileReplies, Pool &pool);

public:
    TaskEval task() override;

public:
    bool handle(MessageBuffer &buffer);

private:
    TaskEval handle();

    void capabilitiesReply();

    void configureNetworkSettings();
    void networkSettingsReply();

    void statusReply();

    void configureIdentity();
    void identityReply();
};

}

#endif
