#ifndef FK_RTC_H_INCLUDED
#define FK_RTC_H_INCLUDED

#include <alogging/sprintf.h>

#include <cstdlib>

#include <RTCZero.h>
#include <RTClib.h>

namespace fk {

class ClockPair {
private:
    RTC_PCF8523 external;
    RTCZero local;

public:
    void begin();

public:
    void setTime(DateTime dt);
    void setTime(uint32_t newTime);
    DateTime now();
    uint32_t getTime();

};

class Clock {
private:
    RTCZero local;

public:
    void begin();

public:
    void setTime(DateTime dt);
    void setTime(uint32_t newTime);
    DateTime now();
    uint32_t getTime();

};

#ifdef FK_RTC_PCF8523
using ClockType = ClockPair;
#else
using ClockType = Clock;
#endif

extern ClockType clock;

inline bool isTimeOff(uint32_t time) {
    // Decided to only do this check if we have a valid time, because if we
    // didn't then what would we do?
    if (time == 0) {
        return true;
    }

    auto now = clock.getTime();
    auto difference = abs((long)(now - time));
    if (difference > 5 * 60) {
        return true;
    }

    return false;
}

template<size_t N>
constexpr size_t length(char const (&)[N]) {
    return N - 1;
}

constexpr size_t MaximumLengthOfTimeString = length("0000/00/00 00:00:00");

inline void timeToString(char *buffer, size_t length, DateTime dt) {
    alogging_snprintf(buffer, length, "%d/%d/%d %02d:%02d:%02d",
                      dt.year(), dt.month(), dt.day(),
                      dt.hour(), dt.minute(), dt.second());
}

class FormattedTime {
private:
    char buffer[MaximumLengthOfTimeString + 1];

public:
    FormattedTime(DateTime dt) {
        timeToString(buffer, sizeof(buffer), dt);
    }

    const char *toString() {
        return buffer;
    }

};

}

#endif
