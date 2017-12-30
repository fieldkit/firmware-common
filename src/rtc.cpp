#include "rtc.h"
#include "debug.h"

namespace fk {

uint32_t clock_set(uint32_t now) {
    Clock clock;
    auto previous = clock.now().unixtime();
    auto diff = now - previous;
    clock.setTime(now);
    if (abs(diff) > 10) {
        FormattedTime nowFormatted{ clock.now() };
        debugfpln("Clock", "Clock changed: %s", nowFormatted.toString());
    }
    return 0;
}

uint32_t clock_now(void) {
    Clock clock;
    auto now = clock.now();
    // TODO: Check for uninitialized clock. This could be better.
    if (now.year() > 2010) {
        return now.unixtime();
    }
    return 0;
}

}
