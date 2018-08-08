#ifndef FK_MODULE_FSM_H_INCLUDED
#define FK_MODULE_FSM_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "pool.h"

#include "tinyfsm.hpp"
#include "state_with_context.h"

namespace fk {

struct ModuleQueryEvent : public tinyfsm::Event {
};

class Pool;
class ModuleInfo;
class Leds;
class Watchdog;
class TwoWireBus;
class ModuleCallbacks;
class TwoWireMessageBuffer;

struct ModuleServices {
    Pool *pool;
    ModuleInfo *info;
    Leds *leds;
    Watchdog *watchdog;
    TwoWireBus *bus;
    ModuleCallbacks *callbacks;
    TwoWireMessageBuffer *outgoing;
    TwoWireMessageBuffer *incoming;
    lws::Writer *writer;

    void clear();
};

class ModuleState;

using ModuleFsm = tinyfsm::Fsm<ModuleState>;

class ModuleState : public ModuleFsm {
public:
    virtual void react(tinyfsm::Event const &ignored);
    virtual void react(ModuleQueryEvent const &ignored);

public:
    virtual void entry() {
        log("Entered");
    }

    virtual void task() {
    }

    virtual void exit() {
    }

public:
    void log(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void info(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void trace(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void warn(const char *f, ...) const __attribute__((format(printf, 2, 3)));
    void error(const char *f, ...) const __attribute__((format(printf, 2, 3)));
};

using ModuleServicesState = StateWithContext<ModuleServices, ModuleState>;

}

#endif
