#include "rtc.h"
#include "debug.h"

namespace fk {

ClockType clock;

constexpr const char LogName[] = "Clock";

using Logger = SimpleLog<LogName>;

void ClockPair::begin() {
    external.begin();
    local.begin();

    auto externalNow = external.now();
    local.setYear(externalNow.year() - 2000);
    local.setMonth(externalNow.month());
    local.setDay(externalNow.day());
    local.setHours(externalNow.hour());
    local.setMinutes(externalNow.minute());
    local.setSeconds(externalNow.second());

    FormattedTime nowFormatted{ externalNow };
    Logger::trace("Synced from external: '%s' (%lu)", nowFormatted.toString(), externalNow.unixtime());
}

void ClockPair::setTime(DateTime dt) {
    external.adjust(dt);
    local.setYear(dt.year() - 2000);
    local.setMonth(dt.month());
    local.setDay(dt.day());
    local.setHours(dt.hour());
    local.setMinutes(dt.minute());
    local.setSeconds(dt.second());

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
    return DateTime(local.getYear(),
                    local.getMonth(),
                    local.getDay(),
                    local.getHours(),
                    local.getMinutes(),
                    local.getSeconds());
}

uint32_t ClockPair::getTime() {
    return now().unixtime();
}

void Clock::begin() {
    local.begin();
}

void Clock::setTime(DateTime dt) {
    local.setYear(dt.year() - 2000);
    local.setMonth(dt.month());
    local.setDay(dt.day());
    local.setHours(dt.hour());
    local.setMinutes(dt.minute());
    local.setSeconds(dt.second());

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
    return DateTime(local.getYear(),
                    local.getMonth(),
                    local.getDay(),
                    local.getHours(),
                    local.getMinutes(),
                    local.getSeconds());
}

uint32_t Clock::getTime() {
    return now().unixtime();
}

}
