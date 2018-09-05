#ifndef FK_GPS_H_INCLUDED
#define FK_GPS_H_INCLUDED

#include <TinyGPS.h>

#include "core_state.h"
#include "serial_port.h"

namespace fk {

class GpsService {
private:
    CoreState *state_;
    SerialPort *serial_;
    TinyGPS gps_;
    bool configured_{ false };
    uint32_t lastStatus_{ 0 };
    uint32_t started_{ 0 };
    uint16_t position_{ 0 };
    uint8_t buffer_[64];
    uint32_t status_{ 0 };

public:
    GpsService(CoreState &state, SerialPort &serial) : state_(&state), serial_(&serial) {
    }

public:
    void read();
    void save();

};

}

#endif
