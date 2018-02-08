#include "core.h"
#include "leds.h"

namespace fk {

constexpr uint32_t GpsFixAttemptInterval = 10 * 1000;
constexpr uint32_t GpsStatusInterval = 1 * 500;

GatherReadings::GatherReadings(TwoWireBus &bus, CoreState &state, Leds &leds, Pool &pool) :
    ActiveObject("GatherReadings"), state(&state), leds(&leds), beginTakeReading(bus, pool, 8), queryReadingStatus(bus, pool, 8) {
}

void GatherReadings::enqueued() {
    if (state->numberOfModules() == 0) {
        log("No attached modules.");
        return;
    }

    leds->beginReading();
    push(beginTakeReading);
}

void GatherReadings::done(Task &task) {
    if (areSame(task, beginTakeReading)) {
        if (beginTakeReading.getBackoff() > 0) {
            log("Using backoff of %lu", beginTakeReading.getBackoff());
            delay.adjust(beginTakeReading.getBackoff());
        } else {
            delay.adjust(300);
        }
        push(delay);
        push(queryReadingStatus);
    } else if (areSame(task, queryReadingStatus)) {
        if (queryReadingStatus.isBusy()) {
            if (queryReadingStatus.getBackoff() > 0) {
                log("Using backoff of %lu", queryReadingStatus.getBackoff());
                delay.adjust(queryReadingStatus.getBackoff());
            } else {
                delay.adjust(300);
            }
            push(delay);
            push(queryReadingStatus);
        } else if (queryReadingStatus.isDone()) {
            state->merge(8, queryReadingStatus.replyMessage());
            push(queryReadingStatus);
        } else {
            leds->doneReading();
        }
    }
}

SendTransmission::SendTransmission(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendTransmission"), builder(&builder), method(&method) {
}

void SendTransmission::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendTransmission::done(Task &task) {
}

SendStatus::SendStatus(TwoWireBus &bus, MessageBuilder &builder, TransmissionTask &method, Pool &pool) :
    ActiveObject("SendStatus"), builder(&builder), method(&method) {
}

void SendStatus::enqueued() {
    method->prepare(*builder);
    push(*method);
}

void SendStatus::done(Task &task) {
}

void ReadGPS::enqueued() {
    started = 0;
    // Ensure we have a fresh start and aren't going to re-use an old fix.
    gps = TinyGPS();
    Serial1.begin(9600);
}

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

TaskEval ReadGPS::task() {
    if (started == 0) {
        started = millis();
    }

    while (Serial1.available()) {
        auto c = (char)Serial1.read();
        gps.encode(c);
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

        state->updateLocation(unix, fix.flon, fix.flat, fix.altitude);
        clock.setTime(dateTime);

        return TaskEval::done();
    }

    if (millis() - started > GpsFixAttemptInterval) {
        state->updateLocationFixFailed();
        return TaskEval::error();
    }

    return TaskEval::idle();
}

}
