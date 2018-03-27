#ifndef FK_APP_MODULE_QUERY_TASK_H_INCLUDED
#define FK_APP_MODULE_QUERY_TASK_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"

namespace fk {

class AppModuleQueryTask : public CustomModuleQueryTask {
private:
    AppReplyMessage *reply;
    AppQueryMessage *query;
    MessageBuffer *buffer;

public:
    AppModuleQueryTask(TwoWireBus &bus, AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer, uint8_t address, Pool &pool);

public:
    void enqueued() override;
    void done() override;
    void error() override;

};

}

#endif
