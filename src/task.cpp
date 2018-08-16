#include <cstdarg>

#include <alogging/alogging.h>

#include "task.h"

namespace fk {

void Task::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name, f, args);
    va_end(args);
}

void Task::info(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::INFO, name, f, args);
    va_end(args);
}

void Task::trace(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::TRACE, name, f, args);
    va_end(args);
}

void Task::warn(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::WARN, name, f, args);
    va_end(args);
}

void Task::error(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    valogf(LogLevels::ERROR, name, f, args);
    va_end(args);
}

}
