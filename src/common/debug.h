#ifndef FK_DEBUG_H_INCLUDED
#define FK_DEBUG_H_INCLUDED

#include <alogging/alogging.h>

const char *firmware_version_get();

void firmware_version_set(const char *hash);

const char *firmware_build_get();

void firmware_build_set(const char *hash);

uint32_t firmware_compiled_get();

void firmware_compiled_set(uint32_t compiled);

uint32_t fk_free_memory();

#define fk_assert(EX) (void)((EX) || (__fk_assert(#EX, __FILE__, __LINE__), 0))

extern "C" void __fk_assert(const char *msg, const char *file, int lineno) __attribute__ ((weak));

#define fk_memzero(ptr, size) memset(ptr, 0, size)

#endif
