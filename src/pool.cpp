#include <Arduino.h>
#include <string.h>
#include <cstdlib>

#include "debug.h"
#include "pool.h"

namespace fk {

Pool::Pool(const char *name, size_t size, Pool *parent) {
    void *block;

    if (parent != nullptr) {
        block = parent->malloc(size);
    }
    else {
        block = ::malloc(size);
    }

    this->name = name;
    this->block = block;
    this->ptr = block;
    this->size = size;
    this->remaining = size;

    debugfpln("Pool", "Create: 0x%x %s size=%d ptr=0x%x (free=%d)", (uint8_t *)this, name, size, ptr, fk_free_memory());
}

void Pool::clear() {
    ptr = block;
    remaining = size;

    debugfpln("Pool", "Clear: 0x%x %s", this, name);
}

void *Pool::malloc(size_t size) {
    auto aligned = size + (4 - (size % 4));

    debugfpln("Pool", "Malloc 0x%x %s size=%d aligned=%d (free=%d)", this, name, size, aligned, remaining - aligned);

    fk_assert(this->size >= aligned);
    fk_assert(this->remaining >= aligned);

    auto *p = ptr;
    ptr = ((uint8_t *)ptr) + aligned;
    remaining -= aligned;

    return (void *)p;
}

Pool::~Pool() {
    debugfpln("Pool", "Free: 0x%x %s", this, name);
    ::free((void *)block);
}

}
