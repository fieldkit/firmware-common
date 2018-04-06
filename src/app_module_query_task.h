#ifndef FK_APP_MODULE_QUERY_TASK_H_INCLUDED
#define FK_APP_MODULE_QUERY_TASK_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"
#include "module_comms.h"

namespace fk {

#if 0
class CustomModuleQueryTask_ : public TwoWireTask {
private:
    AppQueryMessage *appQuery;
    Pool *pool;
    pb_data_t messageData;

public:
    CustomModuleQueryTask_(TwoWireBus &bus, AppQueryMessage &appQuery, Pool &pool, uint8_t address) : TwoWireTask("CustomModuleQuery", bus, pool, address), appQuery(&appQuery), pool(&pool) {
        query.m().type = fk_module_QueryType_QUERY_CUSTOM;
    }

    void enqueued() override {
        TwoWireTask::enqueued();
        query.m().custom.message.arg = appQuery->m().module.message.arg;
    }

};
#endif

class CustomModuleQuery : public ModuleQuery {
private:
    AppReplyMessage *appReply;
    AppQueryMessage *appQuery;
    MessageBuffer *buffer;

public:
    CustomModuleQuery(AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer) : appReply(&reply), appQuery(&query), buffer(&buffer) {
    }

public:
    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_CUSTOM;
        message.m().custom.message.arg = appQuery->m().module.message.arg;
    }

    void reply(ModuleReplyMessage &message) override {
    }

};

class AppModuleQueryTask : public Task {
private:
    AppReplyMessage *reply;
    AppQueryMessage *query;
    MessageBuffer *buffer;
    CustomModuleQuery customModuleQuery;
    ModuleProtocolHandler protocol;

public:
    AppModuleQueryTask(TwoWireBus &bus, AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer, uint8_t address, ModuleCommunications &communications, Pool &pool);

public:
    void enqueued() override;
    TaskEval task() override;
    void done(ModuleProtocolHandler::Finished &finished);
    void error(ModuleProtocolHandler::Finished &finished);

};

}

#endif
