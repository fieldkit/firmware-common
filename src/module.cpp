#include "tuning.h"
#include "module.h"
#include "rtc.h"

namespace fk {

constexpr const char Log[] = "Module";

using Logger = SimpleLog<Log>;

static void module_request_callback() {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->reply();
}

static void module_receive_callback(int bytes) {
    fk_assert(fk::Module::active != nullptr);

    fk::Module::active->receive((size_t)bytes);
}

Module::Module(TwoWireBus &bus, ModuleInfo &info) : bus(&bus),
      outgoing{ bus }, incoming{ bus }, info(&info) {
}

void Module::begin() {
    fk_assert(active == nullptr);

    active = this;

    ModuleServicesState::services(moduleServices);

    fsm_list::start();

    leds.setup();
    watchdog_.setup();
    clock.begin();

    resume();
}

void Module::resume() {
    bus->begin(info->address, module_receive_callback, module_request_callback);
}

void Module::receive(size_t bytes) {
    if (bytes > 0) {
        incoming.clear();
        incoming.readIncoming(bytes);
        pipe.getWriter().write(incoming.ptr(), incoming.position());
    }
}

void Module::reply() {
    if (outgoing.empty()) {
        Logger::info("Sending retry.");
        TwoWireMessageBuffer retryBuffer{ *bus };
        ModuleReplyMessage reply(replyPool);
        reply.m().type = fk_module_ReplyType_REPLY_RETRY;
        retryBuffer.write(reply);
        if (!bus->send(0, retryBuffer.ptr(), retryBuffer.position())) {
            Logger::error("Error sending reply");
        }
        return;
    }

    if (!bus->send(0, outgoing.ptr(), outgoing.position())) {
        Logger::error("Error sending reply");
    }

    // Careful here if this is called after we've placed a message in the
    // outgoing buffer elsewhere. Notice the intentional delay after sending a
    // message before trying to receive one.
    outgoing.clear();
    replyPool.clear();
}

void Module::tick() {
    ModuleState::current().task();
}

void Module::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    loginfof("Module", f, args);
    va_end(args);
}

ModuleReadingStatus Module::beginReading(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

DeferredModuleState Module::beginReadingState() {
    return DeferredModuleState{ };
}

ModuleReadingStatus Module::readingStatus(PendingSensorReading &pending) {
    return ModuleReadingStatus();
}

Module *Module::active{ nullptr };

}
