#include <Wire.h>

#include "module.h"
#include "i2c.h"

namespace fk {

HandleIncoming::HandleIncoming(ModuleInfo *info, MessageBuffer &o, MessageBuffer &i, Pool &pool) :
    Task("HandleIncoming"), info(info), outgoing(o), incoming(i), pool(&pool) {
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
    replyPool("REPLY", 128), handleIncoming { &info, outgoing, incoming, replyPool }, info(&info) {
}

void Module::begin() {
    Wire.begin(info->address);
    Wire.onReceive(module_receive_callback);
    Wire.onRequest(module_request_callback);

    active = this;
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

void Module::beginReading() {
}

void Module::readingDone() {
}

void Module::describeSensor(size_t number) {
}

Module *Module::active { nullptr };

}
