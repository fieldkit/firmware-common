#ifndef FK_DEBUG_H_INCLUDED
#define FK_DEBUG_H_INCLUDED

#include <cstdarg>
#include <cstdint>

typedef void (*debug_hook_fn_t)(const char *str, void *arg);

void debug_add_hook(debug_hook_fn_t hook, void *arg);
void debug_configure_hook(bool enabled);
void debug(const char *str);
void debugf(const char *f, ...);
void vdebugfln(const char *f, va_list args);
void debugfln(const char *f, ...);
void debugfpln(const char *prefix, const char *f, ...);
void vdebugfpln(const char *prefix, const char *f, va_list args);

uint32_t fk_free_memory();

#define fk_assert(EX) (void)((EX) || (__fk_assert(#EX, __FILE__, __LINE__), 0))

void __fk_assert(const char *msg, const char *file, int lineno);

#define fk_memzero(ptr, size) memset(ptr, 0, size)

#endif
