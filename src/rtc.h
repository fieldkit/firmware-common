#ifndef FK_RTC_H_INCLUDED
#define FK_RTC_H_INCLUDED

#include <RTCZero.h>
#include <RTClib.h>

namespace fk {

template<size_t N>
constexpr size_t length(char const (&)[N]) {
    return N - 1;
}

constexpr size_t MaximumLengthOfTimeString = length("0000/00/00 00:00:00");

class Clock {
private:
    char buffer[MaximumLengthOfTimeString + 1];
    bool valid{ false };
    RTCZero rtc;

public:
    void begin() {
        rtc.begin();
        valid = false;
    }

    bool isValid() {
        return valid;
    }

    void setTime(DateTime dt) {
        rtc.setYear(dt.year() - 2000);
        rtc.setMonth(dt.month());
        rtc.setDay(dt.day());
        rtc.setHours(dt.hour());
        rtc.setMinutes(dt.minute());
        rtc.setSeconds(dt.second());
        valid = true;
    }

    void setTime(uint32_t unix) {
        setTime(DateTime(unix));
    }

    DateTime now() {
        return DateTime(rtc.getYear(),
                        rtc.getMonth(),
                        rtc.getDay(),
                        rtc.getHours(),
                        rtc.getMinutes(),
                        rtc.getSeconds());
    }

    uint32_t getTime() {
        return now().unixtime();
    }

    const char *nowString() {
        auto dt = now();
        snprintf(buffer, sizeof(buffer), "%d/%d/%d %02d:%02d:%02d",
                 dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
        return buffer;
    }

};

}

#endif
