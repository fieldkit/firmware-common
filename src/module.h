#ifndef FK_MODULE_H_INCLUDED
#define FK_MODULE_H_INCLUDED

#include "active_object.h"
#include "module_messages.h"
#include "module_servicer.h"
#include "watchdog.h"
#include "leds.h"

namespace fk {

class Module : public ActiveObject, public ModuleCallbacks {
private:
    Pool replyPool;
    MessageBuffer outgoing;
    MessageBuffer incoming;
    ModuleServicer moduleServicer;
    uint32_t lastActivity{ 0 };
    Leds leds;
    Watchdog watchdog{ leds };
    ModuleInfo *info;

public:
    static Module *active;

public:
    Module(ModuleInfo &info);

public:
    void begin();
    void resume();
    void receive(size_t bytes);
    void reply();

public:
    void idle() override;

public:
    ModuleReadingStatus beginReading(SensorReading *readings) override;
    ModuleReadingStatus readingStatus(SensorReading *readings) override;

};

}

#endif
