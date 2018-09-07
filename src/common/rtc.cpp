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
    local_.setYear(externalNow.year() - 2000);
    local_.setMonth(externalNow.month());
    local_.setDay(externalNow.day());
    local_.setHours(externalNow.hour());
    local_.setMinutes(externalNow.minute());
    local_.setSeconds(externalNow.second());

    FormattedTime nowFormatted{ externalNow };
    Logger::trace("Synced from external: '%s' (%lu)", nowFormatted.toString(), externalNow.unixtime());
}

void ClockPair::setTime(DateTime dt) {
    external_.adjust(dt);
    local_.setYear(dt.year() - 2000);
    local_.setMonth(dt.month());
    local_.setDay(dt.day());
    local_.setHours(dt.hour());
    local_.setMinutes(dt.minute());
    local_.setSeconds(dt.second());

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
    return DateTime(local_.getYear(),
                    local_.getMonth(),
                    local_.getDay(),
                    local_.getHours(),
                    local_.getMinutes(),
                    local_.getSeconds());
}

uint32_t ClockPair::getTime() {
    return now().unixtime();
}

void Clock::begin() {
    local_.begin();
}

void Clock::setTime(DateTime dt) {
    local_.setYear(dt.year() - 2000);
    local_.setMonth(dt.month());
    local_.setDay(dt.day());
    local_.setHours(dt.hour());
    local_.setMinutes(dt.minute());
    local_.setSeconds(dt.second());

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
    return DateTime(local_.getYear(),
                    local_.getMonth(),
                    local_.getDay(),
                    local_.getHours(),
                    local_.getMinutes(),
                    local_.getSeconds());
}

uint32_t Clock::getTime() {
    return now().unixtime();
}

}
