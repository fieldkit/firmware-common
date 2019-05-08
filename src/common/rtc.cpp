#include "rtc.h"
#include "debug.h"

namespace fk {

ClockType clock;

constexpr const char ClockPairName[] = "ClockPair";
constexpr const char ZeroClockName[] = "ZeroClock";

using CPLog = SimpleLog<ClockPairName>;
using ZCLog = SimpleLog<ZeroClockName>;

void ClockPair::begin() {
    external_.begin();
    local_.begin();

    auto externalNow = external_.now();
    local_.setEpoch(externalNow.unixtime());

    FormattedTime nowFormatted{ externalNow };
    CPLog::trace("Synced from external: '%s' (%lu)", nowFormatted.toString(), externalNow.unixtime());
}

void ClockPair::setTime(DateTime newEpoch) {
    auto oldEpoch = getTime();

    external_.adjust(newEpoch);
    local_.setEpoch(newEpoch.unixtime());

    auto difference = ((int64_t)newEpoch.unixtime() - oldEpoch);

    if (abs(difference) > 10) {
        FormattedTime newFormatted{ newEpoch };
        FormattedTime oldFormatted{ oldEpoch };
        CPLog::info("Changed: '%s' -> '%s' (%lu) (%" PRId64 "s)", oldFormatted.toString(), newFormatted.toString(), newEpoch.unixtime(), difference);
    }
    else {
        CPLog::info("Ignoring: (%lu) (%" PRId64 "s)", newEpoch.unixtime(), difference);
    }
}

void ClockPair::setTime(uint32_t newTime) {
    if (newTime == 0) {
        CPLog::info("Ignoring invalid time (%lu)", newTime);
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

    auto difference = ((int64_t)newEpoch.unixtime() - oldEpoch);

    if (abs(difference) > 10) {
        FormattedTime newFormatted{ newEpoch };
        FormattedTime oldFormatted{ oldEpoch };
        ZCLog::trace("Changed: '%s' -> '%s' (%lu) (%" PRId64 "s)", oldFormatted.toString(), newFormatted.toString(), newEpoch.unixtime(), difference);
    }
    else {
        ZCLog::info("Ignoring: (%lu) (%" PRId64 "s)", newEpoch.unixtime(), difference);
    }
}

void Clock::setTime(uint32_t newTime) {
    if (newTime == 0) {
        ZCLog::trace("Ignoring invalid time (%lu)", newTime);
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
