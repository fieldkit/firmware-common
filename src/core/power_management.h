#ifndef FK_POWER_H_INCLUDED
#define FK_POWER_H_INCLUDED

#include "task.h"
#include "core_state.h"

namespace fk {

class Power {
private:
    CoreState *state_;
    bool available_{ false };
    bool reliable_{ false };
    uint32_t query_time_{ 0 };
    uint32_t last_alert_{ 0 };
    BatteryStatus status_;

public:
    Power(CoreState &state);

public:
    void setup();
    void task();
    BatteryStatus status() {
        return status_;
    }

};

}

#endif
