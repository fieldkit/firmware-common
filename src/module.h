#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include "active_object.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "watchdog.h"
#include "leds.h"
#include "two_wire.h"
#include "varint_streams.h"

namespace fk {

class Module : public ActiveObject, public ModuleCallbacks {
private:
    TwoWireBus *bus;
    StaticPool<128> replyPool{ "Reply" };
    TwoWireMessageBuffer outgoing;
    TwoWireMessageBuffer incoming;
    ModuleServicer moduleServicer;
    uint32_t lastActivity{ 0 };
    Leds leds;
    Watchdog watchdog{ leds };
    ModuleInfo *info;
    AlignedStorageBuffer<256> scratch;
    CircularStreams<fk::RingBufferN<256>> incomingPipe;
    VarintEncodedStream blockReader{ incomingPipe.getReader(), scratch.toBufferPtr() };

public:
    static Module *active;

public:
    Module(TwoWireBus &bus, ModuleInfo &info);

public:
    virtual void begin();
    void idle() override;

public:
    void resume();
    void receive(size_t bytes);
    void reply();

public:
    ModuleReadingStatus beginReading(PendingSensorReading &pending) override;
    ModuleReadingStatus readingStatus(PendingSensorReading &pending) override;

};

}

#endif
