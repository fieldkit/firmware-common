#include "module_fsm.h"
#include "module_idle.h"

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

template<>
ModuleServices *ModuleServicesState::services_{ nullptr };

void ModuleServices::clear() {
    pool->clear();
}

void ModuleState::react(tinyfsm::Event const &ignored) {
    warn("Ignored Event");
}

void ModuleState::react(ModuleQueryEvent const &ignored) {
    warn("Ignored ModuleQueryEvent");
}

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
