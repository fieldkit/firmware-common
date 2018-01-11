#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include "active_object.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "watchdog.h"
#include "leds.h"
#include "i2c.h"

namespace fk {

class Module : public ActiveObject, public ModuleCallbacks {
private:
    TwoWireBus *bus;
    Pool replyPool;
    TwoWireMessageBuffer outgoing;
    TwoWireMessageBuffer incoming;
    ModuleServicer moduleServicer;
    uint32_t lastActivity{ 0 };
    Leds leds;
    Watchdog watchdog{ leds };
    ModuleInfo *info;

public:
    static Module *active;

public:
    Module(TwoWireBus &bus, ModuleInfo &info);

public:
    void begin();
    void resume();
    void receive(size_t bytes);
    void reply();

public:
    void idle() override;

public:
    ModuleReadingStatus beginReading(PendingSensorReading &pending) override;
    ModuleReadingStatus readingStatus(PendingSensorReading &pending) override;

};

}

#endif
