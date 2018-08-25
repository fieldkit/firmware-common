#ifndef FK_GPS_H_INCLUDED
#define FK_GPS_H_INCLUDED

#include <TinyGPS.h>

#include "task.h"
#include "core_state.h"
#include "serial_port.h"

namespace fk {

class GpsService : public Task {
private:
    CoreState *state;
    SerialPort *serial;
    TinyGPS gps;
    bool configured{ false };
    uint32_t lastStatus{ 0 };
    uint32_t started{ 0 };
    uint16_t position{ 0 };
    uint8_t buffer[64];

public:
    GpsService(CoreState &state, SerialPort &serial) : Task("GPS"), state(&state), serial(&serial) {
    }

public:
    void enqueued() override;
    void read();
    void save();
    TaskEval task() override;

};

}

#endif
