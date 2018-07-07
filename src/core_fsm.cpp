#include "core_fsm.h"
#include "debug.h"

namespace fk {


void CoreDevice::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "CoreDevice", f, args);
    va_end(args);
}

void CoreDevice::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "CoreDevice", f, args);
    va_end(args);
}

void CoreDevice::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::TRACE, "CoreDevice", f, args);
    va_end(args);
}

void CoreDevice::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::WARN, "CoreDevice", f, args);
    va_end(args);
}

}
