#ifndef FK_APP_SERVICER_H_INCLUDED
#define FK_APP_SERVICER_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"
#include "scheduler.h"
#include "fkfs_replies.h"
#include "task_container.h"
#include "app_module_query_task.h"
#include "wifi_client.h"

#include "core_fsm_states.h"

namespace fk {

class AppServicer : public MainServicesState {
private:
    TwoWireBus *bus;
    AppQueryMessage query;
    AppReplyMessage reply;
    CoreState *state;
    Scheduler *scheduler;
    FkfsReplies *fileReplies;
    WifiConnection *connection;
    ModuleCommunications *communications;
    Pool *pool;
    MessageBuffer *buffer{ nullptr };
    uint32_t dieAt{ 0 };
    size_t bytesRead{ 0 };

public:
    AppServicer(TwoWireBus &bus, CoreState &state, Scheduler &scheduler, FkfsReplies &fileReplies, WifiConnection &connection, ModuleCommunications &communications, Pool &pool);

public:
    const char *name() const override {
        return "AppServicer";
    }

public:
    void enqueued();
    void task() override;
    bool service();
    bool flushAndClose();

private:
    bool readQuery();
    bool handle();

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
