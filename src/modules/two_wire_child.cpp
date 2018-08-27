#include "two_wire_child.h"
#include "module_messages.h"
#include "module_fsm.h"

namespace fk {

constexpr const char Log[] = "TwoWireChild";

using Logger = SimpleLog<Log>;

static void module_request_callback() {
    fk_assert(fk::TwoWireChild::active_ != nullptr);

    fk::TwoWireChild::active_->reply();
}

static void module_receive_callback(int bytes) {
    fk_assert(fk::TwoWireChild::active_ != nullptr);

    fk::TwoWireChild::active_->receive((size_t)bytes);
}

TwoWireChild::TwoWireChild(TwoWireBus &bus, uint8_t address) : bus_(&bus), address_(address), outgoing_(bus), incoming_(bus) {
}

void TwoWireChild::setup() {
    fk_assert(active_ == nullptr);

    active_ = this;

    resume();
}

void TwoWireChild::resume() {
    bus_->begin(address_, module_receive_callback, module_request_callback);
}

void TwoWireChild::receive(size_t bytes) {
    if (bytes > 0) {
        incoming_.clear();
        incoming_.readIncoming(bytes);
        auto size = (int32_t)incoming_.position();
        auto wrote = pipe_.getWriter().write(incoming_.ptr(), size);
        if (wrote != size) {
            Logger::info("Dropped (%ld bytes)", size - wrote);
        }
    }
}

void TwoWireChild::reply() {
    if (outgoing_.empty()) {
        auto busy = ModuleState::current().busy();
        Logger::info(busy ? "Busy" : "Retry.");

        TwoWireMessageBuffer retryBuffer{ *bus_ };
        EmptyPool emptyPool;
        ModuleReplyMessage reply(emptyPool);
        reply.m().type = busy ? fk_module_ReplyType_REPLY_BUSY : fk_module_ReplyType_REPLY_RETRY;
        retryBuffer.write(reply);
        if (!bus_->send(0, retryBuffer.ptr(), retryBuffer.position())) {
            Logger::error("Error sending reply");
        }
        return;
    }

    if (!bus_->send(0, outgoing_.ptr(), outgoing_.position())) {
        Logger::error("Error sending reply");
    }

    // Careful here if this is called after we've placed a message in the
    // outgoing buffer elsewhere. Notice the intentional delay after sending a
    // message before trying to receive one.
    outgoing_.clear();
    replyPool_.clear();
}

TwoWireChild *TwoWireChild::active_{ nullptr };

}
