#include "power_management.h"
#include "tuning.h"
#include "debug.h"

namespace fk {

void Power::setup() {
    gauge.powerOn();
}

float Power::percentage() {
    return gauge.stateOfCharge();
}

TaskEval Power::task() {
    if (fk_uptime() > time) {
        time = fk_uptime() + PowerManagementInterval;
        auto percentage = gauge.stateOfCharge();
        auto voltage = gauge.cellVoltage();
        state->updateBattery(percentage, voltage);
    }

    return TaskEval::idle();
}

}
