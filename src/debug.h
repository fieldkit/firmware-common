#ifndef FK_DEBUG_H_INCLUDED
#define FK_DEBUG_H_INCLUDED

#include <Arduino.h>

#include <cstdarg>
#include <cstdint>

typedef size_t (*debug_hook_fn_t)(const char *str, void *arg);

Stream *debug_uart_get();

void debug_uart_set(Stream &standardOut);

void debug_add_hook(debug_hook_fn_t hook, void *arg);

void debug_configure_hook(bool enabled);

void debug(const char *str);

void debugf(const char *f, ...) __attribute__((format(printf, 1, 2)));

void vdebugfln(const char *f, va_list args);

void debugfln(const char *f, ...) __attribute__((format(printf, 1, 2)));

void debugfpln(const char *prefix, const char *f, ...) __attribute__((format(printf, 2, 3)));

void vdebugfpln(const char *prefix, const char *f, va_list args);

uint32_t fk_free_memory();

#define fk_assert(EX) (void)((EX) || (__fk_assert(#EX, __FILE__, __LINE__), 0))

void __fk_assert(const char *msg, const char *file, int lineno);

#define fk_memzero(ptr, size) memset(ptr, 0, size)

const char *firmware_version_get();

void firmware_version_set(const char *hash);

#endif
