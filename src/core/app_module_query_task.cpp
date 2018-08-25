#include "app_module_query_task.h"

namespace fk {

AppModuleQueryTask::AppModuleQueryTask(AppReplyMessage &reply, AppQueryMessage &query, MessageBuffer &buffer, uint8_t address, ModuleCommunications &communications) :
    Task("AppModuleQueryTask"), reply(&reply), query(&query), buffer(&buffer), customModuleQuery(reply, query, buffer), protocol(communications) {
}

void AppModuleQueryTask::enqueued() {
    fk_assert(peripherals.twoWire1().tryAcquire(this));

    protocol.push(8, customModuleQuery);
}

TaskEval AppModuleQueryTask::task() {
    auto finished = protocol.handle();
    if (finished) {
        if (finished.error()) {
            error(finished);
            return TaskEval::error();
        }
        else {
            done(finished);
            return TaskEval::done();
        }
    }
    return TaskEval::idle();
}

void AppModuleQueryTask::done(ModuleProtocolHandler::Finished &finished) {
    reply->clear();

    if (finished.reply->isError()) {
        reply->m().type = fk_app_ReplyType_REPLY_ERROR;
        log("Error reply from module.");
    }
    else {
        reply->m().type = fk_app_ReplyType_REPLY_MODULE;
        reply->m().module.message.funcs.encode = pb_encode_data;
        reply->m().module.message.arg = finished.reply->m().custom.message.arg;
    }

    if (!buffer->write(*reply)) {
        log("Error writing reply");
    }

    peripherals.twoWire1().release(this);
}

void AppModuleQueryTask::error(ModuleProtocolHandler::Finished &finished) {
    reply->m().type = fk_app_ReplyType_REPLY_ERROR;

    if (!buffer->write(*reply)) {
        log("Error writing reply");
    }

    peripherals.twoWire1().release(this);
}

}
