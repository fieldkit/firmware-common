#include "debug.h"
#include "core_fsm.h"
#include "reboot_device.h"
#include "user_wakeup.h"
#include "performance.h"
#include "turn_off.h"

namespace fk {

void CoreDevice::react(tinyfsm::Event const &ignored) {
    warn("Ignored Event");
}

void CoreDevice::react(LowPowerEvent const &ignored) {
    warn("Ignored LowPowerEvent");
}

void CoreDevice::react(SchedulerEvent const &ignored) {
    warn("Ignored SchedulerEvent");
}

void CoreDevice::react(LiveDataEvent const &ignored) {
    warn("Ignored LiveDataEvent");
}

void CoreDevice::react(AppQueryEvent const &ignored) {
}

void CoreDevice::react(UserButtonEvent const &ignored) {
}

void CoreDevice::react(ShortButtonPressEvent const &ignored) {
    warn("ShortButtonPressEvent");
    transit<UserWakeup>();
}

void CoreDevice::react(LongButtonPressEvent const &ignored) {
    warn("LongButtonPressEvent");
    transit<TurnOff>();
}

void CoreDevice::react(ReadingComplete const &ignored) {
}

void CoreDevice::entry() {
    log("Entered");
    Performance::reset();
}

void CoreDevice::task() {
}

void CoreDevice::exit() {
}

void CoreDevice::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void CoreDevice::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void CoreDevice::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::TRACE, name(), f, args);
    va_end(args);
}

void CoreDevice::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::WARN, name(), f, args);
    va_end(args);
}

void CoreDevice::error(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::ERROR, name(), f, args);
    va_end(args);
}

}
