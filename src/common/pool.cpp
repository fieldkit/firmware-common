#include <cstdlib>
#include <string.h>

#include "debug.h"
#include "pool.h"

namespace fk {

Pool::Pool(const char *name, size_t size, void *block) {
    name_ = name;
    block_ = block;
    ptr_ = block;
    size_ = size;
    remaining_ = size;

    #ifdef FK_LOGGING_POOL_VERBOSE
    if (size_ > 0) {
        alogf(LogLevels::TRACE, "Pool", "Create: 0x%p %s size=%d ptr=0x%p (free=%lu)",
              this, name_, size_, ptr_, fk_free_memory());
    }
    #endif
}

void Pool::clear() {
    ptr_ = block_;
    remaining_ = size_;
    frozen_ = false;

    #ifdef FK_LOGGING_POOL_VERBOSE
    alogf(LogLevels::TRACE, "Pool", "Clear: 0x%p %s", this, name_);
    #endif
}

void *Pool::malloc(size_t size) {
    fk_assert(!frozen_);

    auto aligned = alignedSize(size);

    #ifdef FK_LOGGING_POOL_VERBOSE
    alogf(LogLevels::TRACE, "Pool", "Malloc 0x%p %s size=%d aligned=%d (free=%d)",
          this, name_, size, aligned, remaining_ - aligned);
    #endif

    fk_assert(size_ >= aligned);
    fk_assert(remaining_ >= aligned);

    auto *p = ptr_;
    ptr_ = ((uint8_t *)ptr_) + aligned;
    remaining_ -= aligned;

    return (void *)p;
}

void *Pool::copy(void *ptr, size_t size) {
    void *newPtr = malloc(size);
    memcpy(newPtr, ptr, size);
    return newPtr;
}

char *Pool::strdup(const char *str) {
    if (str == nullptr) {
        return nullptr;
    }

    auto length = strlen(str);
    auto ptr = (char *)malloc(length + 1);
    strncpy(ptr, str, length + 1);
    return ptr;
}

Pool Pool::freeze(const char *name) {
    // TODO: Ideally this would keep track of children and warn about
    // allocations on them when we unfreeze.
    frozen_ = true;
    return Pool{ name, remaining_, ptr_ };
}

}
