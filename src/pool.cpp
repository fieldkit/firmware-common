#include <Arduino.h>
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
    debugfpln("Pool", "Create: 0x%x %s size=%d/%d ptr=0x%x (free=%lu)", (unsigned)this, name, size, size, (unsigned)ptr, fk_free_memory());
    #endif
}

void Pool::clear() {
    ptr = block;
    remaining = size;

    #ifdef FK_LOGGING_POOL_VERBOSE
    debugfpln("Pool", "Clear: 0x%x %s", (unsigned)this, name);
    #endif
}

void *Pool::malloc(size_t size) {
    auto aligned = alignedSize(size);

    #ifdef FK_LOGGING_POOL_VERBOSE
    debugfpln("Pool", "Malloc 0x%x %s size=%d aligned=%d (free=%d)", (unsigned)this, name, size, aligned, remaining - aligned);
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

}
