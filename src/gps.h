#ifndef FK_GPS_H_INCLUDED
#define FK_GPS_H_INCLUDED

#include <TinyGPS.h>

#include "active_object.h"
#include "core_state.h"

namespace fk {

class ReadGPS : public Task {
private:
    CoreState *state;
    Uart *uart;
    TinyGPS gps;
    uint32_t lastStatus{ 0 };
    uint32_t started{ 0 };

public:
    ReadGPS(CoreState &state, Uart &uart) : Task("GPS"), state(&state), uart(&uart) {
    }

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
