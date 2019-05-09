#ifndef FK_GPS_H_INCLUDED
#define FK_GPS_H_INCLUDED

#include <TinyGPS.h>

#include "core_state.h"
#include "serial_port.h"

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
    uint32_t chars;

    GpsReading(TinyGPS &gps);

    bool valid();

    DateTime toDateTime();
};

class GpsService {
private:
    CoreState *state_;
    Leds *leds_;
    SerialPort *serial_;
    TinyGPS gps_;
    bool configured_{ false };
    bool disabled_{ false };
    bool initial_{ false };
    uint32_t cleared_{ 0 };
    uint32_t status_{ 0 };
    uint16_t position_{ 0 };
    uint8_t buffer_[64];

public:
    GpsService(CoreState &state, Leds &leds, SerialPort &serial);

public:
    void save();
    void read();

private:
    void save(GpsReading fix);

};

}

#endif
