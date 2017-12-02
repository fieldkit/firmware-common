#include <Wire.h>

#include "module.h"
#include "i2c.h"

namespace fk {

HandleIncoming::HandleIncoming(ModuleInfo *info, ModuleCallbacks &callbacks, MessageBuffer &o, MessageBuffer &i, Pool &pool) :
    Task("HandleIncoming"), info(info), callbacks(&callbacks), outgoing(o), incoming(i), pool(&pool) {
}

void HandleIncoming::read(size_t bytes) {
    fk_assert(incoming.empty());
    for (size_t i = 0; i < bytes; ++i) {
        incoming.append(Wire.read());
    }
}

TaskEval &HandleIncoming::task() {
    QueryMessage query(pool);
    auto status = incoming.read(query);
    incoming.clear();
    if (!status) {
        log("Malformed message");
        return TaskEval::Error;
    }

    fk_assert(outgoing.empty());

    switch (query.m().type) {
    case fk_module_QueryType_QUERY_CAPABILITIES: {
        log("Module info");

        ReplyMessage reply(pool);
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

        ReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_CAPABILITIES;
        reply.m().sensorCapabilities.id = index;
        reply.m().sensorCapabilities.name.arg = (void *)sensor.name;
        reply.m().sensorCapabilities.unitOfMeasure.arg = (void *)sensor.unitOfMeasure;

        outgoing.write(reply);

        break;
    }
    case fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS: {
        log("Begin readings");

        ReplyMessage reply(pool);
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

        ReplyMessage reply(pool);
        reply.m().type = fk_module_ReplyType_REPLY_READING_STATUS;
        reply.m().readingStatus.state = fk_module_ReadingState_IDLE;

        for (size_t i = 0; i < info->numberOfSensors; ++i) {
            if (info->readings[i].status == SensorReadingStatus::Busy) {
                if (reply.m().readingStatus.state != fk_module_ReadingState_DONE) {
                    reply.m().readingStatus.state = fk_module_ReadingState_BUSY;
                }
            }
            if (info->readings[i].status == SensorReadingStatus::Done) {
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
        return TaskEval::Error;
    }
    }

    return TaskEval::Done;
}

static void module_request_callback() {
    fk::Module::active->reply();
}

static void module_receive_callback(int bytes) {
    fk::Module::active->receive((size_t)bytes);
}

Module::Module(ModuleInfo &info) :
    replyPool("REPLY", 128), handleIncoming { &info, *this, outgoing, incoming, replyPool }, info(&info) {
}

void Module::begin() {
    active = this;

    resume();
}

void Module::resume() {
    Wire.begin(info->address);
    Wire.onReceive(module_receive_callback);
    Wire.onRequest(module_request_callback);
}

void Module::receive(size_t bytes) {
    if (bytes > 0) {
        handleIncoming.read(bytes);
        push(handleIncoming);
    }
}

void Module::reply() {
    if (outgoing.empty()) {
        ReplyMessage reply(&replyPool);
        reply.m().type = fk_module_ReplyType_REPLY_RETRY;
        outgoing.write(reply);
    }

    if (!i2c_device_send(0, outgoing.ptr(), outgoing.size())) {
        debugfpln("Module", "Error sending reply");
    }

    // Careful here if this is called after we've placed a message in the
    // outgoing buffer elsewhere. Notice the intentional delay after sending a
    // message before trying to receive one.
    outgoing.clear();
    replyPool.clear();
}

void Module::beginReading(SensorReading *readings) {
}

void Module::readingDone(SensorReading *readings) {
}

Module *Module::active { nullptr };

}
