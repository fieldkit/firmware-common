#include <Arduino.h>
#include <stdarg.h>

#include "debug.h"

const uint32_t FK_DEBUG_LINE_MAX = 128;

Stream *debug_uart = &Serial;
debug_hook_fn_t global_hook_fn = nullptr;
void *global_hook_arg = nullptr;
bool global_hook_enabled = false;

void debug_add_hook(debug_hook_fn_t hook, void *arg) {
    global_hook_fn = hook;
    global_hook_arg = arg;
}

void debug_configure_hook(bool enabled) {
    global_hook_enabled = enabled;
}

Stream *debug_uart_get() {
    return debug_uart;
}

void debug_uart_set(Stream &standardOut) {
    debug_uart = &standardOut;
}

extern "C" {

// Useful for injecting log statements in third party areas for testing.
void fklog(const char *f, ...) {
    char buffer[FK_DEBUG_LINE_MAX];
    va_list args;

    va_start(args, f);
    vsnprintf(buffer, FK_DEBUG_LINE_MAX, f, args);
    va_end(args);

    debug(buffer);
}

}

void debug(const char *str) {
    debug_uart->print(str);
    if (global_hook_fn != nullptr) {
        if (global_hook_enabled) {
            global_hook_enabled = false;
            global_hook_fn(str, global_hook_arg);
            global_hook_enabled = true;
        }
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
#if FK_LOGGING_INCLUDE_CR
    buffer[w] = '\r';
    buffer[w + 1] = '\n';
    buffer[w + 2] = 0;
#else
    buffer[w] = '\n';
    buffer[w + 1] = 0;
#endif

    debug(buffer);
}

void vdebugfpln(const char *prefix, const char *f, va_list args) {
    char messageBuffer[FK_DEBUG_LINE_MAX];
    char timeAndPrefix[6 + 1 + 25 + 1];
    snprintf(timeAndPrefix, sizeof(timeAndPrefix), "%06ld %-25s: ", millis(), prefix);
    vsnprintf(messageBuffer, FK_DEBUG_LINE_MAX, f, args);
    debugfln("%s %s", timeAndPrefix, messageBuffer);
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
    debug_uart->flush();
    while (1) {
        delay(1000);
    }
}
