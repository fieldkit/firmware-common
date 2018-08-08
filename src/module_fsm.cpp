#include "module_fsm.h"

#include "leds.h"
#include "watchdog.h"
#include "module_servicer.h"

namespace fk {

class ModuleBooting : public ModuleServicesState {
public:
    const char *name() const override {
        return "ModuleBooting";
    }

public:
    void task() override {
        transit<ModuleIdle>();
    }

};

void ModuleIdle::task() {
    services().watchdog->task();
    services().moduleServicer->task();
    delay(10);
}

template<>
ModuleServices *ModuleServicesState::services_{ nullptr };

void ModuleState::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void ModuleState::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void ModuleState::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::TRACE, name(), f, args);
    va_end(args);
}

void ModuleState::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::WARN, name(), f, args);
    va_end(args);
}

void ModuleState::error(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::ERROR, name(), f, args);
    va_end(args);
}

}

FSM_INITIAL_STATE(fk::ModuleState, fk::ModuleBooting)
