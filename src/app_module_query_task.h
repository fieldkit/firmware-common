#ifndef FK_APP_MODULE_QUERY_TASK_H_INCLUDED
#define FK_APP_MODULE_QUERY_TASK_H_INCLUDED

#include "active_object.h"
#include "app_messages.h"
#include "core_state.h"

namespace fk {

class AppModuleQueryTask : public ActiveObject {
private:
    AppReplyMessage *reply;
    AppQueryMessage *query;
    MessageBuffer *buffer;
    CustomModuleQueryTask customModuleQueryTask;

public:
    struct Context {
        TwoWireBus &bus;
        AppReplyMessage &reply;
        AppQueryMessage &query;
        MessageBuffer &buffer;
        uint8_t address;
        Pool &pool;
    };

public:
    AppModuleQueryTask(Context c);

public:
    void enqueued() override;
    void done(Task &task) override;
    void error(Task &task) override;

};

}

#endif
