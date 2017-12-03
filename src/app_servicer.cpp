#include "app_servicer.h"

namespace fk {

AppServicer::AppServicer(ModuleController &modules, Pool &pool)
    : Task("AppServicer"), query(&pool), modules(&modules), pool(&pool) {
}

TaskEval AppServicer::task() {
    handle(query);
    return TaskEval::done();
}

bool AppServicer::read(MessageBuffer &buffer) {
    return buffer.read(query);
}

void AppServicer::handle(AppQueryMessage &query) {
    switch (query.m().type) {
    case fk_app_QueryType_QUERY_CAPABILITIES: {
        log("Query caps");

        AppReplyMessage reply(pool);
        reply.m().type = fk_app_ReplyType_REPLY_CAPABILITIES;
        reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        outgoing.write(reply);

        break;
    }
    case fk_app_QueryType_QUEYR_CONFIGURE_SENSOR: {
    }
    case fk_app_QueryType_QUERY_DATA_SETS: {
    }
    case fk_app_QueryType_QUERY_DATA_SET: {
    }
    case fk_app_QueryType_QUERY_DOWNLOAD_DATA_SET: {
    }
    case fk_app_QueryType_QUERY_ERASE_DATA_SET: {
    }
    case fk_app_QueryType_QUERY_LIVE_DATA_POLL: {
    }
    default: {
        AppReplyMessage reply(pool);
        reply.error("Unknown query");
        outgoing.write(reply);

        break;
    }
    }
}

}
