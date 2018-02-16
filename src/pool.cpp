#include <Arduino.h>
#include <cstdlib>
#include <string.h>

#include "debug.h"
#include "pool.h"

namespace fk {

inline size_t alignedSize(size_t size) {
    return size + (4 - (size % 4));
}

Pool::Pool(const char *name, size_t size, Pool *parent) {
    void *block;

    auto aligned = alignedSize(size);

    if (parent != nullptr) {
        block = parent->malloc(aligned);
    } else {
        block = ::malloc(aligned);
    }

    this->name = name;
    this->block = block;
    this->ptr = block;
    this->size = aligned;
    this->remaining = aligned;

    debugfpln("Pool", "Create: 0x%x %s size=%d/%d ptr=0x%x (free=%lu)", (unsigned)this, name, size, aligned, (unsigned)ptr, fk_free_memory());
}

void Pool::clear() {
    ptr = block;
    remaining = size;

    debugfpln("Pool", "Clear: 0x%x %s", (unsigned)this, name);
}

void *Pool::malloc(size_t size) {
    auto aligned = alignedSize(size);

    debugfpln("Pool", "Malloc 0x%x %s size=%d aligned=%d (free=%d)", (unsigned)this, name, size, aligned, remaining - aligned);

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

Pool::~Pool() {
    debugfpln("Pool", "Free: 0x%x %s", (unsigned)this, name);
    ::free((void *)block);
}

}
