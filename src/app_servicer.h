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
#include "wifi_client.h"

namespace fk {

class AppServicer : public Task {
private:
    TwoWireBus *bus;
    AppQueryMessage query;
    AppReplyMessage reply;
    LiveData *liveData;
    CoreState *state;
    Scheduler *scheduler;
    FkfsReplies *fileReplies;
    WifiConnection *connection;
    ModuleCommunications *communications;
    Pool *pool;

    TaskContainer<AppModuleQueryTask> appModuleQueryTask;
    MessageBuffer *buffer{nullptr};
    ChildContainer active;
    uint32_t dieAt{ 0 };
    size_t bytesRead{ 0 };

public:
    AppServicer(TwoWireBus &bus, LiveData &liveData, CoreState &state, Scheduler &scheduler, FkfsReplies &fileReplies, WifiConnection &connection, ModuleCommunications &communications, Pool &pool);

public:
    void enqueued() override;
    TaskEval task() override;
    void done() override;
    void error() override;

private:
    TaskEval handle();

private:
    bool readQuery();
    bool flushAndClose();

private:
    void capabilitiesReply();

    void configureNetworkSettings();
    void networkSettingsReply();

    void statusReply();

    void configureIdentity();
    void identityReply();
};

}

#endif
