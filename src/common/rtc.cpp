#include "rtc.h"
#include "debug.h"

namespace fk {

ClockType clock;

constexpr const char LogName[] = "Clock";

using Logger = SimpleLog<LogName>;

void ClockPair::begin() {
    external_.begin();
    local_.begin();

    auto externalNow = external_.now();
    local_.setEpoch(externalNow.unixtime());

    FormattedTime nowFormatted{ externalNow };
    Logger::trace("(ClockPair) Synced from external: '%s' (%lu)", nowFormatted.toString(), externalNow.unixtime());
}

void ClockPair::setTime(DateTime newEpoch) {
    auto oldEpoch = getTime();

    external_.adjust(newEpoch);
    local_.setEpoch(newEpoch.unixtime());

    FormattedTime newFormatted{ newEpoch };
    FormattedTime oldFormatted{ oldEpoch };
    Logger::trace("(ClockPair) Changed: '%s' -> '%s' (%lu) (%" PRId64 "s)", oldFormatted.toString(), newFormatted.toString(),
                  newEpoch.unixtime(), ((int64_t)newEpoch.unixtime() - oldEpoch));
}

void ClockPair::setTime(uint32_t newTime) {
    if (newTime == 0) {
        Logger::trace("(ClockPair) Ignoring invalid time (%lu)", newTime);
        return;
    }
    setTime(DateTime(newTime));
}

DateTime ClockPair::now() {
    return DateTime(local_.getEpoch());
}

uint32_t ClockPair::getTime() {
    return now().unixtime();
}

void Clock::begin() {
    local_.begin();
}

void Clock::setTime(DateTime newEpoch) {
    auto oldEpoch = getTime();

    local_.setEpoch(newEpoch.unixtime());

    FormattedTime newFormatted{ newEpoch };
    FormattedTime oldFormatted{ oldEpoch };
    Logger::trace("(ZeroClock) Changed: '%s' -> '%s' (%lu) (%" PRId64 "s)", oldFormatted.toString(), newFormatted.toString(),
                  newEpoch.unixtime(), ((int64_t)newEpoch.unixtime() - oldEpoch));
}

void Clock::setTime(uint32_t newTime) {
    if (newTime == 0) {
        Logger::trace("(ZeroClock) Ignoring invalid time (%lu)", newTime);
        return;
    }

    setTime(DateTime(newTime));
}

DateTime Clock::now() {
    return DateTime(local_.getEpoch());
}

uint32_t Clock::getTime() {
    return now().unixtime();
}

}
