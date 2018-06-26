#include "hardware.h"
#include "gps.h"
#include "tuning.h"

namespace fk {

struct GpsReading {
    uint8_t satellites;

    float flon;
    float flat;
    float altitude;
    uint32_t positionFixAge;
    float course;
    float speed;
    uint32_t hdop;

    uint32_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t hundredths;
    uint32_t timeFixAge;
    uint32_t date;
    uint32_t time;

    GpsReading(TinyGPS &gps) {
        satellites = gps.satellites();
        gps.f_get_position(&flat, &flon, &positionFixAge);
        hdop = gps.hdop();
        altitude = gps.f_altitude();
        course = gps.f_course();
        speed = gps.f_speed_kmph();
        gps.crack_datetime((int *)&year, &month, &day, &hour, &minute, &second, &hundredths, &timeFixAge);
        gps.get_datetime(&date, &time, &timeFixAge);
    }

    bool isValid() {
        if (satellites == TinyGPS::GPS_INVALID_SATELLITES) return false;
        if (flon == TinyGPS::GPS_INVALID_ANGLE) return false;
        if (flat == TinyGPS::GPS_INVALID_ANGLE) return false;
        if (altitude == TinyGPS::GPS_INVALID_ALTITUDE) return false;
        if (positionFixAge == TinyGPS::GPS_INVALID_AGE) return false;
        if (course == TinyGPS::GPS_INVALID_ANGLE) return false;
        if (speed == TinyGPS::GPS_INVALID_SPEED) return false;
        if (hdop == TinyGPS::GPS_INVALID_HDOP) return false;
        if (hdop == TinyGPS::GPS_INVALID_HDOP) return false;
        if (timeFixAge == TinyGPS::GPS_INVALID_AGE) return false;
        if (date == TinyGPS::GPS_INVALID_DATE) return false;
        if (time == TinyGPS::GPS_INVALID_TIME) return false;

        return true;
    }

    DateTime toDateTime() {
        return DateTime(year, month, day, hour, minute, second);
    }
};

void ReadGps::enqueued() {
    started = 0;
    // Ensure we have a fresh start and aren't going to re-use an old fix.
    gps = TinyGPS();

    serial->begin(9600);
}

constexpr const char *PGCMD_ANTENNA = "$PGCMD,33,1*6C";
constexpr const char *PMTK_SET_NMEA_UPDATE_1HZ = "$PMTK220,1000*1F";
constexpr const char *PMTK_API_SET_FIX_CTL_1HZ = "$PMTK300,1000,0,0,0,0*1C";

TaskEval ReadGps::task() {
    if (started == 0) {
        position = 0;
        started = millis();

        if (!configured) {
            serial->println(PGCMD_ANTENNA);
            serial->println(PMTK_SET_NMEA_UPDATE_1HZ);
            serial->println(PMTK_API_SET_FIX_CTL_1HZ);
            configured = true;
        }
    }

    while (serial->available()) {
        auto c = (char)serial->read();
        gps.encode(c);
        if (GpsEchoRaw) {
            if (c == '\n' || c == '\r' || position == sizeof(buffer) - 1) {
                if (position > 0 && buffer[0] == '$') {
                    buffer[position] = 0;
                    log("GPS: %s", buffer);
                }
                position = 0;
            }
            else {
                buffer[position++] = c;
            }
        }
    }

    if (millis() - lastStatus < GpsStatusInterval) {
        lastStatus = millis();
        return TaskEval::idle();
    }

    auto fix = GpsReading{ gps };
    if (fix.isValid()) {
        auto dateTime = fix.toDateTime();
        auto unix = dateTime.unixtime();

        log("Time(%lu) Sats(%d) Hdop(%lu) Loc(%f, %f, %f)", unix, fix.satellites, fix.hdop, fix.flon, fix.flat, fix.altitude);

        state->updateLocation(DeviceLocation{ unix, fix.flon, fix.flat, fix.altitude });
        clock.setTime(dateTime);

        return TaskEval::done();
    }

    if (millis() - started > GpsFixAttemptInterval) {
        state->updateLocation(DeviceLocation{});
        return TaskEval::error();
    }

    return TaskEval::idle();
}

}
