#include "app_module_query_task.h"

namespace fk {

AppModuleQueryTask::AppModuleQueryTask(TwoWireBus &bus, AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer, uint8_t address, Pool &pool) :
    ActiveObject("AppModuleQuery"), reply(&reply), query(&query), buffer(&buffer), customModuleQueryTask(bus, pool, address) {
}

void AppModuleQueryTask::enqueued() {
    customModuleQueryTask.ready(*query);
    push(customModuleQueryTask);
}

void AppModuleQueryTask::done(Task &task) {
    if (areSame(task, customModuleQueryTask)) {
        reply->clear();

        if (customModuleQueryTask.replyMessage().isError()) {
            reply->m().type = fk_app_ReplyType_REPLY_ERROR;
            log("Error reply from module.");
        }
        else {
            reply->m().type = fk_app_ReplyType_REPLY_MODULE;
            reply->m().module.message.funcs.encode = pb_encode_data;
            reply->m().module.message.arg = customModuleQueryTask.replyMessage().m().custom.message.arg;
        }

        if (!buffer->write(*reply)) {
            log("Error writing reply");
        }
    }

    peripherals.twoWire1().release();
}

void AppModuleQueryTask::error(Task &task) {
    reply->m().type = fk_app_ReplyType_REPLY_ERROR;
    if (!buffer->write(*reply)) {
        log("Error writing reply");
    }

    peripherals.twoWire1().release();
}

}
