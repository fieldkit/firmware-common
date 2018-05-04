#include "rtc.h"
#include "debug.h"

namespace fk {

ClockType clock;

static void log(const char *f, ...) {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::TRACE, "Clock", f, args);
    va_end(args);
}

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
    log("Synced from external: %s", nowFormatted.toString());

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
        log("Ignoring invalid time (%lu)", newTime);
        return;
    }

    setTime(DateTime(newTime));

    FormattedTime nowFormatted{ now() };
    log("Clock changed: %s (%lu)", nowFormatted.toString(), newTime);
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
        log("Ignoring invalid time (%lu)", newTime);
        return;
    }

    setTime(DateTime(newTime));

    FormattedTime nowFormatted{ now() };
    log("Clock changed: %s (%lu)", nowFormatted.toString(), newTime);
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
