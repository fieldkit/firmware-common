#ifndef FK_RTC_H_INCLUDED
#define FK_RTC_H_INCLUDED

#include <RTCZero.h>
#include <RTClib.h>

#include "hardware.h"

namespace fk {

template<size_t N>
constexpr size_t length(char const (&)[N]) {
    return N - 1;
}

constexpr size_t MaximumLengthOfTimeString = length("0000/00/00 00:00:00");

inline void timeToString(char *buffer, size_t length, DateTime dt) {
    snprintf(buffer, length, "%d/%d/%d %02d:%02d:%02d",
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

class Clock {
private:
    bool valid{ false };
    #ifdef FK_RTC_PCF8523
    RTC_PCF8523 rtc;
    #else
    RTCZero rtc;
    #endif

public:
    void begin();
    bool isValid() {
        return valid;
    }

public:
    void setTime(DateTime dt);
    void setTime(uint32_t newTime);
    DateTime now();
    uint32_t getTime();

};

extern Clock clock;

}

#endif
