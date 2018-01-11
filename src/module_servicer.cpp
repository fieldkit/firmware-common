#include "module_servicer.h"
#include "module.h"
#include "rtc.h"

namespace fk {

ModuleServicer::ModuleServicer(TwoWireBus &bus, ModuleInfo &info, ModuleCallbacks &callbacks, TwoWireMessageBuffer &o, TwoWireMessageBuffer &i, Pool &pool)
    : Task("ModuleServicer"), bus(&bus), info(&info), callbacks(&callbacks), outgoing(&o), incoming(&i), pool(&pool) {
}

void ModuleServicer::read(size_t bytes) {
    incoming->readIncoming(bytes);
}

TaskEval ModuleServicer::task() {
    ModuleQueryMessage query(*pool);
    auto status = incoming->read(query);
    incoming->clear();
    if (!status) {
        log("Malformed message");
        return TaskEval::error();
    }

    fk_assert(outgoing->empty());

    if (!handle(query)) {
        return TaskEval::error();
    }

    return TaskEval::done();
}

bool ModuleServicer::handle(ModuleQueryMessage &query) {
    switch (query.m().type) {
    case fk_module_QueryType_QUERY_CAPABILITIES: {
        log("Module info (%lu)", query.m().beginTakeReadings.callerTime);

        clock.setTime(query.m().queryCapabilities.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES;
        reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        reply.m().capabilities.type = fk_module_ModuleType_SENSOR;
        reply.m().capabilities.name.arg = (void *)info->name;
        reply.m().capabilities.numberOfSensors = info->numberOfSensors;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_SENSOR_CAPABILITIES: {
        auto index = query.m().querySensorCapabilities.sensor;

        log("Sensor #%d: info", index);

        SensorInfo &sensor = info->sensors[index];

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_SENSOR_CAPABILITIES;
        reply.m().sensorCapabilities.id = index;
        reply.m().sensorCapabilities.name.arg = (void *)sensor.name;
        reply.m().sensorCapabilities.unitOfMeasure.arg = (void *)sensor.unitOfMeasure;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS: {
        log("Begin readings (%lu)", query.m().beginTakeReadings.callerTime);

        clock.setTime(query.m().beginTakeReadings.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_BEGIN;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            info->readings[i].status = SensorReadingStatus::Busy;
        }

        pending.elapsed = 0;
        pending.readings = info->readings;
        auto status = callbacks->beginReading(pending);
        if (status.backoff > 0) {
            reply.m().readingStatus.backoff = status.backoff;
        }

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_READING_STATUS: {
        log("Reading status");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_IDLE;

        pending.readings = info->readings;
        auto status = callbacks->readingStatus(pending);
        if (status.backoff > 0) {
            reply.m().readingStatus.backoff = status.backoff;
        }

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            if (info->readings[i].status == SensorReadingStatus::Busy) {
                if (reply.m().readingStatus.state != fk_module_ReadingState_DONE) {
                    reply.m().readingStatus.state = fk_module_ReadingState_BUSY;
                }
            }
            if (info->readings[i].status == SensorReadingStatus::Done) {
                log("Sending reading");
                reply.m().readingStatus.state = fk_module_ReadingState_DONE;
                reply.m().readingStatus.elapsed = pending.elapsed;
                reply.m().sensorReading.sensor = i;
                reply.m().sensorReading.time = info->readings[i].time;
                reply.m().sensorReading.value = info->readings[i].value;
                info->readings[i].status = SensorReadingStatus::Idle;
                break;
            }
        }

        outgoing->write(reply);

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
