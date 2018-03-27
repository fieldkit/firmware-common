#include "power_management.h"
#include "tuning.h"
#include "debug.h"

namespace fk {

void Power::setup() {
    gauge.powerOn();
}

TaskEval Power::task() {
    if (millis() > time) {
        time = millis() + PowerManagementInterval;
        auto percentage = gauge.stateOfCharge();
        auto voltage = gauge.cellVoltage();
        state->updateBattery(percentage, voltage);
    }

    return TaskEval::idle();
}

}
