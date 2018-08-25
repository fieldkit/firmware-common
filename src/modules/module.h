#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "task.h"
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
    ModuleInfo *info;
    TwoWireMessageBuffer outgoing;
    TwoWireMessageBuffer incoming;
    StaticPool<128> replyPool{ "Reply" };
    Leds leds;
    Watchdog watchdog_{ leds };
    SerialFlashFileSystem flashFs_{ watchdog_ };
    FlashState<MinimumFlashState> flashState_{ flashFs_ };
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
        &pipe.getReader(),
        &flashFs_,
        &flashState_
    };

public:
    static Module *active;

public:
    Module(TwoWireBus &bus, ModuleInfo &info);

public:
    virtual void begin();
    virtual void tick();

    void run();
    void setupFlash();

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
