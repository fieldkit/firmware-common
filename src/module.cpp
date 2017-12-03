#include <Wire.h>

#include "i2c.h"
#include "module.h"

namespace fk {

static void module_request_callback() {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->reply();
}

static void module_receive_callback(int bytes) {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->receive((size_t)bytes);
}

Module::Module(ModuleInfo &info)
    : replyPool("REPLY", 128), handleIncoming{ &info, *this, outgoing, incoming, replyPool }, info(&info) {
}

void Module::begin() {
    fk_assert(active == nullptr);

    active = this;

    resume();
}

void Module::resume() {
    i2c_begin(info->address, module_receive_callback, module_request_callback);
}

void Module::receive(size_t bytes) {
    if (bytes > 0) {
        handleIncoming.read(bytes);
        push(handleIncoming);
    }
}

void Module::reply() {
    if (outgoing.empty()) {
        ModuleReplyMessage reply(replyPool);
        reply.m().type = fk_module_ReplyType_REPLY_RETRY;
        outgoing.write(reply);
    }

    if (!i2c_device_send(0, outgoing.ptr(), outgoing.position())) {
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

Module *Module::active{ nullptr };

}
