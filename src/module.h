#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#define LWS_ENABLE_PROTOBUF
#include <lwstreams/lwstreams.h>

#include "active_object.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "watchdog.h"
#include "leds.h"
#include "two_wire.h"

#include "module_fsm.h"

namespace fk {

class Module : public ModuleCallbacks {
private:
    TwoWireBus *bus;
    StaticPool<128> replyPool{ "Reply" };
    TwoWireMessageBuffer outgoing;
    TwoWireMessageBuffer incoming;
    uint32_t lastActivity{ 0 };
    Leds leds;
    Watchdog watchdog_{ leds };
    ModuleInfo *info;
    lws::CircularStreams<lws::RingBufferN<256>> pipe;
    ModuleServices moduleServices{
        &replyPool,
        info,
        &leds,
        &watchdog_,
        bus,
        this,
        &outgoing,
        &incoming,
        &pipe,
        &pipe.getWriter(),
        &pipe.getReader()
    };

public:
    static Module *active;

public:
    Module(TwoWireBus &bus, ModuleInfo &info);

public:
    virtual void begin();
    virtual void tick();

    void run();

public:
    Watchdog &watchdog() {
        return watchdog_;
    }

public:
    void resume();
    void receive(size_t bytes);
    void reply();
    void log(const char *f, ...) const;

public:
    uint32_t elapsedSinceActivity() const {
        return fk_uptime() - lastActivity;
    }

public:
    ModuleReadingStatus beginReading(PendingSensorReading &pending) override;
    ModuleReadingStatus readingStatus(PendingSensorReading &pending) override;
    DeferredModuleState beginReadingState() override;

};

using fsm_list = tinyfsm::FsmList<ModuleState>;

template<typename E>
void send_event(E const & event) {
    fsm_list::template dispatch<E>(event);
}

}

#endif
