#include "app_servicer.h"

namespace fk {

AppServicer::AppServicer(const char *name, ModuleController &modules, CoreState &state, Pool &pool)
    : Task(name), query(&pool), modules(&modules), state(&state), pool(&pool) {
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

        auto *attached = state->attachedModules();
        auto numberOfSensors = state->numberOfSensors();
        auto sensorIndex = 0;
        fk_app_SensorCapabilities sensors[numberOfSensors];
        for (size_t moduleIndex = 0; attached[moduleIndex].address > 0; ++moduleIndex) {
            for (size_t i = 0; i < attached[moduleIndex].numberOfSensors; ++i) {
                sensors[sensorIndex].id = i;
                sensors[sensorIndex].name.funcs.encode = pb_encode_string;
                sensors[sensorIndex].name.arg = (void *)attached[moduleIndex].sensors[i].name;
                sensors[sensorIndex].unitOfMeasure.funcs.encode = pb_encode_string;
                sensors[sensorIndex].unitOfMeasure.arg = (void *)attached[moduleIndex].sensors[i].unitOfMeasure;
                sensors[sensorIndex].frequency = 60;

                log("%d / %d: %s", sensorIndex, numberOfSensors, sensors[sensorIndex].name.arg);

                sensorIndex++;
            }
        }

        pb_array_t sensors_array = {
            .length = numberOfSensors,
            .itemSize = sizeof(fk_app_SensorCapabilities),
            .buffer = sensors,
            .fields = fk_app_SensorCapabilities_fields,
        };

        AppReplyMessage reply(pool);
        reply.m().type = fk_app_ReplyType_REPLY_CAPABILITIES;
        reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        reply.m().capabilities.name.funcs.encode = pb_encode_string;
        reply.m().capabilities.name.arg = (void *)"NOAA-CTD";
        reply.m().capabilities.sensors.funcs.encode = pb_encode_array;
        reply.m().capabilities.sensors.arg = (void *)&sensors_array;
        if (!outgoing.write(reply)) {
            log("Error writing reply");
        }

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
