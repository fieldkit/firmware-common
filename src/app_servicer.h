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

#include "api_connection.h"

namespace fk {

class AppServicer : public ApiConnection {
private:
    CoreState *state_;
    FkfsReplies *fileReplies_;
    ModuleCommunications *communications_;
    Pool *pool_;

public:
    AppServicer(CoreState &state, FkfsReplies &fileReplies, WifiConnection &connection, ModuleCommunications &communications, Pool &pool);

public:
    const char *name() const override {
        return "AppServicer";
    }

private:
    bool handle() override;

private:
    void capabilitiesReply();

    void configureNetworkSettings();
    void networkSettingsReply();

    void statusReply();

    void configureIdentity();
    void identityReply();

    void metadataReply();
};

}

#endif
