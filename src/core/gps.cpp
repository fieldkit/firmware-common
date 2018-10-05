#include "gps.h"
#include "hardware.h"
#include "configuration.h"

namespace fk {

constexpr const char Log[] = "GPS";

using Logger = SimpleLog<Log>;

constexpr const char *PGCMD_ANTENNA = "$PGCMD,33,1*6C";
constexpr const char *PMTK_SET_NMEA_UPDATE_1HZ = "$PMTK220,1000*1F";
constexpr const char *PMTK_API_SET_FIX_CTL_1HZ = "$PMTK300,1000,0,0,0,0*1C";

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

    bool valid() {
        if (satellites == TinyGPS::GPS_INVALID_SATELLITES) return false;
        if (satellites > configuration.gps.required_satellites) return false;

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

void GpsService::read() {
    if (disabled_) {
        return;
    }

    if (!configured_) {
        gps_ = TinyGPS();
        serial_->begin(9600);
        serial_->println(PGCMD_ANTENNA);
        serial_->println(PMTK_SET_NMEA_UPDATE_1HZ);
        serial_->println(PMTK_API_SET_FIX_CTL_1HZ);

        configured_ = true;
        position_ = 0;
        cleared_ = fk_uptime();
        status_ = fk_uptime();

        Logger::info("Configured");
    }

    if (configuration.gps.clear_interval > 0 && fk_uptime() - cleared_ > configuration.gps.clear_interval) {
        gps_ = TinyGPS();
        cleared_ = fk_uptime();
    }

    if (configuration.gps.status_interval > 0 && fk_uptime() - status_ > configuration.gps.status_interval) {
        auto fix = GpsReading{ gps_ };
        auto unix = fix.toDateTime().unixtime();
        Logger::log("Time(%lu) Sats(%d) Hdop(%lu) Loc(%f, %f, %f)", unix, fix.satellites, fix.hdop, fix.flon, fix.flat, fix.altitude);
        status_ = fk_uptime();
    }

    while (serial_->available()) {
        auto c = (char)serial_->read();
        gps_.encode(c);

        if (configuration.gps.echo) {
            if (c == '\n' || c == '\r' || position_ == sizeof(buffer_) - 1) {
                if (position_ > 0 && buffer_[0] == '$') {
                    buffer_[position_] = 0;
                    Logger::trace("GPS: %s", buffer_);
                }
                position_ = 0;
            }
            else {
                buffer_[position_++] = c;
            }
        }
    }

    if (!initial_) {
        auto fix = GpsReading{ gps_ };
        if (fix.valid()) {
            save();
            initial_ = false;
        }
    }
}

void GpsService::save() {
    if (disabled_) {
        return;
    }

    auto fix = GpsReading{ gps_ };
    if (fix.valid()) {
        auto dateTime = fix.toDateTime();
        auto unix = dateTime.unixtime();

        Logger::log("Time(%lu) Sats(%d) Hdop(%lu) Loc(%f, %f, %f)", unix, fix.satellites, fix.hdop, fix.flon, fix.flat, fix.altitude);

        state_->updateLocation(DeviceLocation{ unix, fix.flon, fix.flat, fix.altitude });

        clock.setTime(dateTime);

        if (configuration.gps.station_fixed) {
            Hardware::disableGps();
            disabled_ = true;
        }
    }
    else {
        state_->updateLocation(DeviceLocation{});
        Logger::log("No fix");
    }
}

}
