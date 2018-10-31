#include "debug.h"
#include "tuning.h"
#include "power_management.h"
#include "core_fsm.h"

namespace fk {

void Power::setup() {
    gauge_.powerOn();
}

float Power::percentage() {
    return gauge_.stateOfCharge();
}

float Power::voltage() {
    return gauge_.cellVoltage();
}

TaskEval Power::task() {
    if (fk_uptime() > queryTime_) {
        queryTime_ = fk_uptime() + PowerManagementQueryInterval;
        auto percentage = gauge_.stateOfCharge();
        auto voltage = gauge_.cellVoltage();
        state_->updateBattery(percentage, voltage);

        if (fk_uptime() - lastAlert_ > PowerManagementAlertInterval) {
            if (percentage < BatteryLowPowerSleepThreshold) {
                send_event(LowPowerEvent{ });
            }
            lastAlert_ = fk_uptime();
        }
    }

    return TaskEval::idle();
}

}
