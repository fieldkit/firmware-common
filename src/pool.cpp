#include <cstdlib>
#include <string.h>

#include "debug.h"
#include "pool.h"

namespace fk {

Pool::Pool(const char *name, size_t size, void *block) {
    this->name = name;
    this->block = block;
    this->ptr = block;
    this->size = size;
    this->remaining = size;

    #ifdef FK_LOGGING_POOL_VERBOSE
    if (size > 0) {
        logf(LogLevels::TRACE, "Pool", "Create: 0x%p %s size=%d/%d ptr=0x%p (free=%lu)",
             this, name, size, size, ptr, fk_free_memory());
    }
    #endif
}

void Pool::clear() {
    ptr = block;
    remaining = size;

    #ifdef FK_LOGGING_POOL_VERBOSE
    logf(LogLevels::TRACE, "Pool", "Clear: 0x%p %s", this, name);
    #endif
}

void *Pool::malloc(size_t size) {
    auto aligned = alignedSize(size);

    #ifdef FK_LOGGING_POOL_VERBOSE
    logf(LogLevels::TRACE, "Pool", "Malloc 0x%p %s size=%d aligned=%d (free=%d)",
         this, name, size, aligned, remaining - aligned);
    #endif

    fk_assert(this->size >= aligned);
    fk_assert(this->remaining >= aligned);

    auto *p = ptr;
    ptr = ((uint8_t *)ptr) + aligned;
    remaining -= aligned;

    return (void *)p;
}

void *Pool::copy(void *ptr, size_t size) {
    void *newPtr = malloc(size);
    memcpy(newPtr, ptr, size);
    return newPtr;
}

char *Pool::strdup(const char *str) {
    auto length = strlen(str);
    auto ptr = (char *)malloc(length);
    strncpy(ptr, str, length);
    return ptr;
}

}
