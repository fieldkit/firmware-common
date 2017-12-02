#include "module_servicer.h"
#include "module.h"

namespace fk {

ModuleServicer::ModuleServicer(ModuleInfo *info, ModuleCallbacks &callbacks, MessageBuffer &o, MessageBuffer &i, Pool &pool) :
    Task("ModuleServicer"), info(info), callbacks(&callbacks), outgoing(o), incoming(i), pool(&pool) {
}

void ModuleServicer::read(size_t bytes) {
    incoming.read(bytes);
}

TaskEval &ModuleServicer::task() {
    ModuleQueryMessage query(pool);
    auto status = incoming.read(query);
    incoming.clear();
    if (!status) {
        log("Malformed message");
        return TaskEval::Error;
    }

    fk_assert(outgoing.empty());

    if (!handle(query)) {
        return TaskEval::Error;
    }

    return TaskEval::Done;
}

bool ModuleServicer::handle(ModuleQueryMessage &query) {
    switch (query.m().type) {
    case fk_module_QueryType_QUERY_CAPABILITIES: {
        log("Module info");

        ModuleReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES; reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        reply.m().capabilities.type = fk_module_ModuleType_SENSOR;
        reply.m().capabilities.name.arg = (void *)info->name;
        reply.m().capabilities.numberOfSensors = info->numberOfSensors;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_SENSOR_CAPABILITIES: {
        auto index = query.m().querySensorCapabilities.sensor;

        log("Sensor #%d: info", index);

        SensorInfo &sensor  = info->sensors[index];

        ModuleReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES;
        reply.m().sensorCapabilities.id = index;
        reply.m().sensorCapabilities.name.arg = (void *)sensor.name;
        reply.m().sensorCapabilities.unitOfMeasure.arg = (void *)sensor.unitOfMeasure;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS: {
        log("Begin readings");

        ModuleReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_BEGIN;

        outgoing.write(reply);

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            info->readings[i].status = SensorReadingStatus::Busy;
        }

        callbacks->beginReading(info->readings);

        break;
    }
    case fk_module_QueryType_QUERY_READING_STATUS: {
        log("Reading status");

        ModuleReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_IDLE;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            if (info->readings[i].status == SensorReadingStatus::Busy) {
                if (reply.m().readingStatus.state != fk_module_ReadingState_DONE) {
                    reply.m().readingStatus.state = fk_module_ReadingState_BUSY;
                }
            }
            if (info->readings[i].status == SensorReadingStatus::Done) {
                log("Sending reading");
                reply.m().readingStatus.state = fk_module_ReadingState_DONE;
                reply.m().sensorReading.sensor = info->readings[i].sensor;
                reply.m().sensorReading.time = info->readings[i].time;
                reply.m().sensorReading.value = info->readings[i].value;
                info->readings[i].status = SensorReadingStatus::Idle;
                break;
            }
        }

        outgoing.write(reply);

        break;
    }
    default: {
        log("Unknown query: %d", query.m().type);
        return false;
    }
    }

    return true;
}

}
