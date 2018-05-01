#include <Wire.h>

#include "tuning.h"
#include "module.h"
#include "rtc.h"

namespace fk {

static void module_request_callback() {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->reply();
}

static void module_receive_callback(int bytes) {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->receive((size_t)bytes);
}

Module::Module(TwoWireBus &bus, ModuleInfo &info) : bus(&bus),
      outgoing{ bus }, incoming{ bus }, moduleServicer{ bus, info, *this, outgoing, incoming, incomingPipe.getWriter(), replyPool }, info(&info) {
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
    bus->begin(info->address, module_receive_callback, module_request_callback);
}

void Module::receive(size_t bytes) {
    if (bytes > 0) {
        lastActivity = millis();
        moduleServicer.read(bytes);
        servicing.append(moduleServicer);
    }
}

void Module::reply() {
    if (outgoing.empty()) {
        log("Sending retry.");
        TwoWireMessageBuffer retryBuffer{ *bus };
        ModuleReplyMessage reply(replyPool);
        reply.m().type = fk_module_ReplyType_REPLY_RETRY;
        retryBuffer.write(reply);
        if (!bus->send(0, retryBuffer.ptr(), retryBuffer.position())) {
            log("Error sending reply");
        }
        return;
    }

    if (!bus->send(0, outgoing.ptr(), outgoing.position())) {
        log("Error sending reply");
    }

    // Careful here if this is called after we've placed a message in the
    // outgoing buffer elsewhere. Notice the intentional delay after sending a
    // message before trying to receive one.
    outgoing.clear();
    replyPool.clear();
}

void Module::tick() {
    watchdog.task();
    servicing.task();

    if (millis() - lastActivity > IdleRebootInterval) {
        log("Reboot due to inactivity.");
        NVIC_SystemReset();
    }

    auto block = blockReader.read();
    if (block.eos()) {
        log("End of stream.");
    }
    if (block) {
        log("Read %ld bytes", block.blockSize);
    }
}

void Module::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "Module", f, args);
    va_end(args);
}

ModuleReadingStatus Module::beginReading(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

ModuleReadingStatus Module::readingStatus(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

Module *Module::active{ nullptr };

}
