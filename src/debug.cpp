#include <Arduino.h>
#include <stdarg.h>

#include "debug.h"

constexpr uint32_t FK_DEBUG_LINE_MAX = 128;

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

    debug_uart->print(buffer);
}

}

void debug_raw(const fk_log_message_t *m) {
    char buffer[FK_DEBUG_LINE_MAX * 2];
    auto pos = snprintf(buffer, sizeof(buffer) - 3, "%06ld %-25s ", m->uptime, m->facility);
    auto len = strlen(m->message);
    memcpy(buffer + pos, m->message, len);
    pos += len;
    #if FK_LOGGING_INCLUDE_CR
    buffer[pos + 0] = '\r';
    buffer[pos + 1] = '\n';
    buffer[pos + 2] = 0;
    #else
    buffer[pos + 0] = '\n';
    buffer[pos + 1] = 0;
    #endif
    debug_uart->print(buffer);

    if (global_hook_fn != nullptr) {
        if (global_hook_enabled) {
            global_hook_enabled = false;
            global_hook_fn(m, buffer, global_hook_arg);
            global_hook_enabled = true;
        }
    }
}

void vdebugfpln(LogLevels level, const char *facility, const char *f, va_list args) {
    char messageBuffer[FK_DEBUG_LINE_MAX];
    vsnprintf(messageBuffer, FK_DEBUG_LINE_MAX, f, args);

    fk_log_message_t m = {
        .uptime = millis(),
        .time = 0,
        .level = (uint8_t)level,
        .facility = facility,
        .message = messageBuffer,
    };

    debug_raw(&m);
}

void debug_log(LogLevels level, const char *prefix, const char *f, ...) {
    va_list args;
    va_start(args, f);
    vdebugfpln(level, prefix, f, args);
    va_end(args);
}

void debugfpln(const char *prefix, const char *f, ...) {
    va_list args;
    va_start(args, f);
    vdebugfpln(LogLevels::INFO, prefix, f, args);
    va_end(args);
}

const char *global_firmware_build = "<unknown>";

const char *firmware_build_get() {
    return global_firmware_build;
}

void firmware_build_set(const char *hash) {
    global_firmware_build = hash;
}

const char *global_firmware_version = "<unknown>";

const char *firmware_version_get() {
    return global_firmware_version;
}

void firmware_version_set(const char *hash) {
    global_firmware_version = hash;
}

extern "C" char *sbrk(int32_t i);

uint32_t fk_free_memory() {
    char stack_dummy = 0;
    return &stack_dummy - sbrk(0);
}

void __fk_assert(const char *msg, const char *file, int lineno) {
    debugfpln("Assert", "ASSERTION: %s:%d '%s'", file, lineno, msg);
    debug_uart->flush();

    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    while (1) {
        // This will happen until the WDT kicks is back to Reset, hopefully. I'm
        // flashing these just to sort of give the user an idea that something's
        // going wrong if they happen to be around.
        digitalWrite(A3, LOW);
        digitalWrite(A4, LOW);
        digitalWrite(A5, LOW);
        delay(100);
        digitalWrite(A3, HIGH);
        digitalWrite(A4, HIGH);
        digitalWrite(A5, HIGH);
        delay(100);
    }
}
