#ifndef FK_STATIC_WIFI_ALLOCATOR_H_INCLUDED
#define FK_STATIC_WIFI_ALLOCATOR_H_INCLUDED

#include <WiFiSocket.h>

namespace fk {

class StaticWiFiAllocator : public WiFiAllocator {
    static constexpr size_t NumberOfBuffers = 2;
    typename std::aligned_storage<sizeof(uint8_t) * WifiSocketBufferSize, alignof(uint8_t)>::type data[NumberOfBuffers];
    bool available[NumberOfBuffers] = { true, true };

public:
    void *malloc(size_t size) override {
        fk_assert(size == WifiSocketBufferSize);

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

    void free(void *ptr) override {
        for (size_t i = 0; i < NumberOfBuffers; ++i) {
            void *mem = (void *)&data[i];
            if (mem == ptr) {
                fk_assert(!available[i]);
                available[i] = true;
            }
        }
    }

};

}

#endif
