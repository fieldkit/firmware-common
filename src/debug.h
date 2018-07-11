#ifndef FK_DEBUG_H_INCLUDED
#define FK_DEBUG_H_INCLUDED

#include <alogging/alogging.h>

const char *firmware_version_get();

void firmware_version_set(const char *hash);

const char *firmware_build_get();

void firmware_build_set(const char *hash);

uint32_t fk_free_memory();

#define fk_assert(EX) (void)((EX) || (__fk_assert(#EX, __FILE__, __LINE__), 0))

void __fk_assert(const char *msg, const char *file, int lineno) __attribute__ ((weak));

#define fk_memzero(ptr, size) memset(ptr, 0, size)

namespace fk {

// NOTE: This is a C++11 feature.
template<const char *Name>
class SimpleLog {
public:
    static void log(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::INFO, Name, f, args);
        va_end(args);
    }

    static void info(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::INFO, Name, f, args);
        va_end(args);
    }

    static void trace(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::TRACE, Name, f, args);
        va_end(args);
    }

    static void warn(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::WARN, Name, f, args);
        va_end(args);
    }

    static void error(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vlogf(LogLevels::ERROR, Name, f, args);
        va_end(args);
    }

};

}

#endif
