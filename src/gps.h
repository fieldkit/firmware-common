#ifndef FK_GPS_H_INCLUDED
#define FK_GPS_H_INCLUDED

#include <TinyGPS.h>

#include "active_object.h"
#include "core_state.h"
#include "hardware.h"

namespace fk {

class ReadGps : public Task {
private:
    CoreState *state;
    SerialPort *serial;
    TinyGPS gps;
    uint32_t lastStatus{ 0 };
    uint32_t started{ 0 };
    uint16_t position{ 0 };
    uint8_t buffer[128];

public:
    ReadGps(CoreState &state, SerialPort &serial) : Task("GPS"), state(&state), serial(&serial) {
    }

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
