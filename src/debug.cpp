#include <Arduino.h>
#include <stdarg.h>

#include "debug.h"

const uint32_t FK_DEBUG_LINE_MAX = 128;

debug_hook_fn_t global_hook_fn = nullptr;
void *global_hook_arg = nullptr;

void debug_add_hook(debug_hook_fn_t hook, void *arg) {
    global_hook_fn = hook;
    global_hook_arg = arg;
}

void debug(const char *str) {
    Serial.print(str);
    if (global_hook_fn != nullptr) {
        global_hook_fn(str, global_hook_arg);
    }
}

void debugf(const char *f, ...) {
    char buffer[FK_DEBUG_LINE_MAX];
    va_list args;

    va_start(args, f);
    vsnprintf(buffer, FK_DEBUG_LINE_MAX, f, args);
    va_end(args);

    debug(buffer);
}

void vdebugfln(const char *f, va_list args) {
    char buffer[FK_DEBUG_LINE_MAX];

    auto w = vsnprintf(buffer, FK_DEBUG_LINE_MAX - 2, f, args);
    buffer[w    ] = '\r';
    buffer[w + 1] = '\n';
    buffer[w + 2] = 0;

    debug(buffer);
}

void vdebugfpln(const char *prefix, const char *f, va_list args) {
    debugf("%06d %-25s: ", millis(), prefix);
    vdebugfln(f, args);
}

void debugfpln(const char *prefix, const char *f, ...) {
    va_list args;
    va_start(args, f);
    vdebugfpln(prefix, f, args);
    va_end(args);
}

void debugfln(const char *f, ...) {
    va_list args;
    va_start(args, f);
    vdebugfln(f, args);
    va_end(args);
}

extern "C" char *sbrk(int32_t i);

uint32_t fk_free_memory() {
    char stack_dummy = 0;
    return &stack_dummy - sbrk(0);
}

void __fk_assert(const char *msg, const char *file, int lineno) {
    debugfln("ASSERTION: %s:%d '%s'", file, lineno, msg);
    Serial.flush();
    while (1) {
        delay(1000);
    }
}
