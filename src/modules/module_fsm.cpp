#include "module_fsm.h"
#include "module_idle.h"
#include "startup.h"
#include "watchdog.h"
#include "hardware.h"

namespace fk {

template<>
ModuleServices *ModuleServicesState::services_{ nullptr };

void ModuleServices::alive() {
    watchdog->task();
    hardware->task();
    if (hooks != nullptr) {
        hooks->alive();
    }
}

void ModuleServices::clear() {
    pool->clear();
}

void ModuleState::react(tinyfsm::Event const &e) {
    warn("Ignored Event");
}

void ModuleState::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void ModuleState::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void ModuleState::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::TRACE, name(), f, args);
    va_end(args);
}

void ModuleState::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::WARN, name(), f, args);
    va_end(args);
}

void ModuleState::error(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::ERROR, name(), f, args);
    va_end(args);
}

}

FSM_INITIAL_STATE(fk::ModuleState, fk::Booting)
