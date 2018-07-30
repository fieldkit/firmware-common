#include "debug.h"
#include "core_fsm.h"
#include "reboot_device.h"
#include "user_wakeup.h"

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
    warn("Ignored AppQueryEvent");
}

void CoreDevice::react(UserButtonEvent const &ignored) {
}

void CoreDevice::react(UserWakeupEvent const &ignored) {
    warn("UserWakeupEvent");
    transit<UserWakeup>();
}

void CoreDevice::react(UserRebootEvent const &ignored) {
    warn("UserRebootEvent");
    transit<RebootDevice>();
}

void CoreDevice::react(ReadingComplete const &ignored) {
}

void CoreDevice::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void CoreDevice::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, name(), f, args);
    va_end(args);
}

void CoreDevice::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::TRACE, name(), f, args);
    va_end(args);
}

void CoreDevice::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::WARN, name(), f, args);
    va_end(args);
}

void CoreDevice::error(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::ERROR, name(), f, args);
    va_end(args);
}

}
