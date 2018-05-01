#include "rtc.h"
#include "debug.h"

namespace fk {

ClockType clock;

void ClockPair::begin() {
    external.begin();
    local.begin();
}

void ClockPair::setTime(DateTime dt) {
    external.adjust(dt);
    local.setYear(dt.year() - 2000);
    local.setMonth(dt.month());
    local.setDay(dt.day());
    local.setHours(dt.hour());
    local.setMinutes(dt.minute());
    local.setSeconds(dt.second());
    valid = true;
}

void ClockPair::setTime(uint32_t newTime) {
    if (newTime == 0) {
        loginfof("Clock", "Ignoring invalid time (%lu)", newTime);
        return;
    }

    setTime(DateTime(newTime));

    FormattedTime nowFormatted{ now() };
    loginfof("Clock", "Clock changed: %s (%lu)", nowFormatted.toString(), newTime);
}

DateTime ClockPair::now() {
    // auto e = external.now();
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
    valid = false;
}

void Clock::setTime(DateTime dt) {
    local.setYear(dt.year() - 2000);
    local.setMonth(dt.month());
    local.setDay(dt.day());
    local.setHours(dt.hour());
    local.setMinutes(dt.minute());
    local.setSeconds(dt.second());
    valid = true;
}

void Clock::setTime(uint32_t newTime) {
    if (newTime == 0) {
        loginfof("Clock", "Ignoring invalid time (%lu)", newTime);
        return;
    }

    setTime(DateTime(newTime));

    FormattedTime nowFormatted{ now() };
    loginfof("Clock", "Clock changed: %s (%lu)", nowFormatted.toString(), newTime);
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
