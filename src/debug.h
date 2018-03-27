#ifndef FK_DEBUG_H_INCLUDED
#define FK_DEBUG_H_INCLUDED

#include <Arduino.h>

#include <cstdarg>
#include <cstdint>

typedef struct fk_log_message_t {
    uint32_t uptime;
    uint32_t time;
    uint8_t level;
    const char *facility;
    const char *message;
} fk_log_message_t;

typedef size_t (*debug_hook_fn_t)(const fk_log_message_t *m, const char *formatted, void *arg);

Stream *debug_uart_get();

void debug_uart_set(Stream &standardOut);

void debug_add_hook(debug_hook_fn_t hook, void *arg);

void debug_configure_hook(bool enabled);

void debuglog(const fk_log_message_t *m);

void debugfpln(const char *prefix, const char *f, ...) __attribute__((format(printf, 2, 3)));

void vdebugfpln(const char *prefix, const char *f, va_list args);

const char *firmware_version_get();

void firmware_version_set(const char *hash);

uint32_t fk_free_memory();

#define fk_assert(EX) (void)((EX) || (__fk_assert(#EX, __FILE__, __LINE__), 0))

void __fk_assert(const char *msg, const char *file, int lineno);

#define fk_memzero(ptr, size) memset(ptr, 0, size)

#endif
