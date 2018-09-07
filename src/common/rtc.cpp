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
    Logger::trace("Synced from external: '%s' (%lu)", nowFormatted.toString(), externalNow.unixtime());
}

void ClockPair::setTime(DateTime dt) {
    external_.adjust(dt);
    local_.setEpoch(dt.unixtime());

    FormattedTime nowFormatted{ dt };
    Logger::trace("Clock changed: '%s' (%lu)", nowFormatted.toString(), dt.unixtime());
}

void ClockPair::setTime(uint32_t newTime) {
    if (newTime == 0) {
        Logger::trace("Ignoring invalid time (%lu)", newTime);
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

void Clock::setTime(DateTime dt) {
    auto epoch = dt.unixtime();
    local_.setEpoch(epoch);

    FormattedTime nowFormatted{ dt };
    Logger::trace("Clock changed: '%s' (%lu)", nowFormatted.toString(), dt.unixtime());
}

void Clock::setTime(uint32_t newTime) {
    if (newTime == 0) {
        Logger::trace("Ignoring invalid time (%lu)", newTime);
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
