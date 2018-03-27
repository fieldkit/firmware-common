#include "app_module_query_task.h"

namespace fk {

AppModuleQueryTask::AppModuleQueryTask(TwoWireBus &bus, AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer, uint8_t address, Pool &pool) :
    CustomModuleQueryTask(bus, pool, address), reply(&reply), query(&query), buffer(&buffer) {
}

void AppModuleQueryTask::enqueued() {
    ready(*query);
    CustomModuleQueryTask::enqueued();
}

void AppModuleQueryTask::done() {
    reply->clear();

    if (replyMessage().isError()) {
        reply->m().type = fk_app_ReplyType_REPLY_ERROR;
        log("Error reply from module.");
    }
    else {
        reply->m().type = fk_app_ReplyType_REPLY_MODULE;
        reply->m().module.message.funcs.encode = pb_encode_data;
        reply->m().module.message.arg = replyMessage().m().custom.message.arg;
    }

    if (!buffer->write(*reply)) {
        log("Error writing reply");
    }

    peripherals.twoWire1().release(this);
}

void AppModuleQueryTask::error() {
    reply->m().type = fk_app_ReplyType_REPLY_ERROR;

    if (!buffer->write(*reply)) {
        log("Error writing reply");
    }

    peripherals.twoWire1().release(this);
}

}
