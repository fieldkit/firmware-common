#include "module_servicer.h"
#include "module.h"
#include "rtc.h"

namespace fk {

ModuleServicer::ModuleServicer(TwoWireBus &bus, ModuleInfo &info, ModuleCallbacks &callbacks, TwoWireMessageBuffer &o, TwoWireMessageBuffer &i, lws::Writer &writer, Pool &pool)
    : Task("ModuleServicer"), bus(&bus), info(&info), callbacks(&callbacks), outgoing(&o), incoming(&i), writer(&writer), pool(&pool) {
}

void ModuleServicer::read(size_t bytes) {
    incoming->readIncoming(bytes);
    auto wrote = writer->write(incoming->ptr(), incoming->position());
    if (false) {
        log("Read %d bytes (%ld)", bytes, wrote);
    }
}

TaskEval ModuleServicer::task() {
    ModuleQueryMessage query(*pool);
    auto status = incoming->read(query);
    incoming->clear();
    if (!status) {
        log("Malformed message");
        return TaskEval::error();
    }

    if (!outgoing->empty()) {
        log("Orphaned reply! QueryType=%d ReplySize=%d", outgoing->position(), query.m().type);
        outgoing->clear();
    }

    return handle(query);
}

TaskEval ModuleServicer::handle(ModuleQueryMessage &query) {
    switch (query.m().type) {
    case fk_module_QueryType_QUERY_CAPABILITIES: {
        log("Module info (%lu)", query.m().beginTakeReadings.callerTime);

        clock.setTime(query.m().queryCapabilities.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES;
        reply.m().capabilities.version = FK_MODULE_PROTOCOL_VERSION;
        reply.m().capabilities.type = (fk_module_ModuleType)info->type;
        reply.m().capabilities.name.arg = (void *)info->name;
        reply.m().capabilities.numberOfSensors = info->numberOfSensors;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_SENSOR_CAPABILITIES: {
        auto index = query.m().querySensorCapabilities.sensor;

        log("Sensor #%lu: info", index);

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
        log("Begin readings (%lu) (%lu)",
            query.m().beginTakeReadings.callerTime,
            query.m().beginTakeReadings.number
        );

        clock.setTime(query.m().beginTakeReadings.callerTime);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_BEGIN;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            info->readings[i].status = SensorReadingStatus::Busy;
        }

        pending.number = query.m().beginTakeReadings.number;
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
        log("Reading status (%lu)", query.m().queryReadingStatus.sleep);

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
                if (reply.m().readingStatus.state == fk_module_ReadingState_BUSY) {
                    reply.m().readingStatus.state = fk_module_ReadingState_BUSY;
                }
            }
            if (info->readings[i].status == SensorReadingStatus::Done) {
                log("Sending reading %d", i);
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
    case fk_module_QueryType_QUERY_CUSTOM: {
        log("Custom message");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_ERROR;

        callbacks->message(query, reply);

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_DATA_APPEND: {
        log("Data (%lu)", query.m().data.size);

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_DATA;
        reply.m().data.size = 4096;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_DATA_CLEAR: {
        log("Data clear");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_DATA;
        reply.m().data.size = 0;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TRANSMISSION: {
        log("Begin transmission");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_TRANSMISSION_STATUS;
        reply.m().transmissionStatus.state = fk_module_TransmissionState_TRANSMISSION_IDLE;

        outgoing->write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_TRANSMISSION_STATUS: {
        log("Transmission status");

        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_TRANSMISSION_STATUS;
        reply.m().transmissionStatus.state = fk_module_TransmissionState_TRANSMISSION_IDLE;

        outgoing->write(reply);

        break;
    }
    default: {
        log("Unknown query: %d", query.m().type);
        ModuleReplyMessage reply(*pool);
        reply.m().type = fk_module_ReplyType_REPLY_ERROR;
        if (!outgoing->write(reply)) {
            log("Error writing reply");
        }
        break;
    }
    }

    return TaskEval::done();
}

}
