#include "wifi_callbacks.h"
#include "watchdog.h"
#include "platform.h"
#include "debug.h"

namespace fk {

void *StaticWiFiCallbacks::malloc(size_t size) {
    fk_assert(size == WifiSocketBufferSize);

    // logtracef("SWA", "Malloc");

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

void StaticWiFiCallbacks::free(void *ptr) {
    // logtracef("SWA", "Free");

    for (size_t i = 0; i < NumberOfBuffers; ++i) {
        void *mem = (void *)&data[i];
        if (mem == ptr) {
            fk_assert(!available[i]);
            available[i] = true;
        }
    }
}

bool StaticWiFiCallbacks::busy(uint32_t elapsed) {
    if (fk_uptime() - ticked > 1000) {
        ticked = fk_uptime();

        if (elapsed > 0) {
            logtracef("Wifi", "Busy (%lu)", elapsed);
        }
    }

    Watchdog::checkin();

    return true;
}

}
