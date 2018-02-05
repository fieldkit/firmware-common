#include "core.h"
#include "leds.h"

namespace fk {

constexpr uint32_t GpsFixAttemptInterval = 10 * 1000;
constexpr uint32_t GpsStatusInterval = 1 * 1000;

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
    started = millis();
    Serial1.begin(9600);
}

TaskEval ReadGPS::task() {
    if (millis() - started > GpsFixAttemptInterval) {
        log("No GPS fix.");
        state->updateLocationFixFailed();
        return TaskEval::error();
    }

    while (Serial1.available()) {
        auto c = (char)Serial1.read();
        gps.encode(c);
    }

    if (millis() - lastStatus > GpsStatusInterval) {
        float flat, flon;
        uint32_t positionAage;
        gps.f_get_position(&flat, &flon, &positionAage);

        auto satellites = gps.satellites();
        auto hdop = gps.hdop();
        auto altitude = gps.f_altitude();

        auto year = 0;
        uint8_t month, day, hour, minute, second, hundredths;
        uint32_t age;
        gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
        DateTime dateTime(year, month, day, hour, minute, second);

        log("Sats(%d) Hdop(%lu) Loc(%f, %f) Alt(%f)", satellites, hdop, flon, flat, altitude);

        if (flon != TinyGPS::GPS_INVALID_F_ANGLE && flat != TinyGPS::GPS_INVALID_F_ANGLE && altitude != TinyGPS::GPS_INVALID_F_ALTITUDE) {
            state->updateLocation(dateTime.unixtime(), flon, flat, altitude);
            clock.setTime(dateTime);
            return TaskEval::done();
        }

        lastStatus = millis();
    }

    return TaskEval::idle();
}

DetermineLocation::DetermineLocation(TwoWireBus &bus, CoreState &state, Pool &pool) :
    ActiveObject("DetermineLocation"), state(&state), readGps(bus, state) {
}

void DetermineLocation::enqueued() {
    push(readGps);
}

void DetermineLocation::done(Task &task) {
}

}
