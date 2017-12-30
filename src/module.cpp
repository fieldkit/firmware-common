#include <Wire.h>

#include "i2c.h"
#include "module.h"
#include "rtc.h"

namespace fk {

constexpr uint32_t IdleRebootInterval = 1000 * 60 * 10;
constexpr uint32_t ModuleServicingMemory = 128;

static void module_request_callback() {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->reply();
}

static void module_receive_callback(int bytes) {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->receive((size_t)bytes);
}

Module::Module(ModuleInfo &info)
    : ActiveObject(info.name), replyPool("REPLY", ModuleServicingMemory),
      moduleServicer{ &info, *this, outgoing, incoming, replyPool }, info(&info) {
}

void Module::begin() {
    fk_assert(active == nullptr);

    active = this;

    leds.setup();
    watchdog.setup();
    clock.begin();

    resume();
}

void Module::resume() {
    i2c_begin(info->address, module_receive_callback, module_request_callback);
}

void Module::receive(size_t bytes) {
    if (bytes > 0) {
        lastActivity = millis();
        moduleServicer.read(bytes);
        push(moduleServicer);
    }
}

void Module::reply() {
    if (outgoing.empty()) {
        ModuleReplyMessage reply(replyPool);
        reply.m().type = fk_module_ReplyType_REPLY_RETRY;
        outgoing.write(reply);
    }

    if (!i2c_device_send(0, outgoing.ptr(), outgoing.position())) {
        log("Error sending reply");
    }

    // Careful here if this is called after we've placed a message in the
    // outgoing buffer elsewhere. Notice the intentional delay after sending a
    // message before trying to receive one.
    outgoing.clear();
    replyPool.clear();
}

void Module::idle() {
    watchdog.tick();

    if (millis() - lastActivity > IdleRebootInterval) {
        log("Reboot due to inactivity.");
        NVIC_SystemReset();
    }
}

ModuleReadingStatus Module::beginReading(SensorReading *) {
    return ModuleReadingStatus();
}

ModuleReadingStatus Module::readingStatus(SensorReading *readings) {
    return ModuleReadingStatus();
}

Module *Module::active{ nullptr };

}
