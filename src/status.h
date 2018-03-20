#ifndef FK_STATUS_H_INCLUDED
#define FK_STATUS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire.h"
#include "utils.h"

namespace fk {

class Status : public ActiveObject {
private:
    uint32_t lastTick{ 0 };
    CoreState *state;
    TwoWireBus *bus;

public:
    Status(CoreState &state, TwoWireBus &bus) : ActiveObject("Status"), state(&state), bus(&bus) {
    }

public:
    void idle() override {
        if (millis() - lastTick > 5000) {
            IpAddress4 ip{ state->getStatus().ip };
            auto now = clock.now();
            debugfpln("Status", "Status %" PRIu32 " (%.2f%% / %.2fmv) (%" PRIu32 " free) (%s) (%s)", now.unixtime(),
                      state->getStatus().batteryPercentage, state->getStatus().batteryVoltage,
                      fk_free_memory(), ip.toString(), deviceId.toString());
            lastTick = millis();
        }
    }

};

}

#endif
