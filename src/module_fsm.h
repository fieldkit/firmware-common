#ifndef FK_MODULE_FSM_H_INCLUDED
#define FK_MODULE_FSM_H_INCLUDED

#include "tinyfsm.hpp"
#include "state_with_context.h"

namespace fk {

class Leds;
class Watchdog;
class ModuleServicer;

struct ModuleServices {
    Leds *leds;
    Watchdog *watchdog;
    ModuleServicer *moduleServicer;

    void clear() {
    }
};

class ModuleState;

using ModuleFsm = tinyfsm::Fsm<ModuleState>;

class ModuleState : public ModuleFsm {
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

class ModuleIdle : public ModuleServicesState {
public:
    const char *name() const override {
        return "ModuleIdle";
    }

public:
    void task() override;

};

}

#endif
