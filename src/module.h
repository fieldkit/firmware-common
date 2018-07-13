#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#define LWS_ENABLE_PROTOBUF
#include <lwstreams/lwstreams.h>

#include "active_object.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "watchdog.h"
#include "leds.h"
#include "two_wire.h"

namespace fk {

class Module : public ModuleCallbacks {
private:
    TwoWireBus *bus;
    StaticPool<128> replyPool{ "Reply" };
    Supervisor<5> servicing{ true };
    TwoWireMessageBuffer outgoing;
    TwoWireMessageBuffer incoming;
    ModuleServicer moduleServicer;
    uint32_t lastActivity{ 0 };
    Leds leds;
    Watchdog watchdog_{ leds };
    ModuleInfo *info;
    lws::AlignedStorageBuffer<256> scratch;
    lws::CircularStreams<lws::RingBufferN<256>> incomingPipe;
    lws::VarintEncodedStream blockReader{ incomingPipe.getReader(), scratch.toBufferPtr() };

public:
    static Module *active;

public:
    Module(TwoWireBus &bus, ModuleInfo &info);

public:
    virtual void begin();
    virtual void tick();

public:
    TaskQueue &taskQueue() {
        return servicing;
    }

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

};

}

#endif
