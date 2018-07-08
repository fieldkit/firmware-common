#include "static_wifi_allocator.h"
#include "debug.h"

namespace fk {

void *StaticWiFiAllocator::malloc(size_t size) {
    fk_assert(size == WifiSocketBufferSize);

    logf(LogLevels::TRACE, "SWA", "Malloc");

    for (size_t i = 0; i < NumberOfBuffers; ++i) {
        if (available[i]) {
            void *mem = (void *)&data[i];
            available[i] = false;
            return mem;
        }
    }

    fk_assert(false);
    return nullptr;
}

void StaticWiFiAllocator::free(void *ptr) {
    logf(LogLevels::TRACE, "SWA", "Free");

    for (size_t i = 0; i < NumberOfBuffers; ++i) {
        void *mem = (void *)&data[i];
        if (mem == ptr) {
            fk_assert(!available[i]);
            available[i] = true;
        }
    }
}

}
