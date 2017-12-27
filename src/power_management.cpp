#include "power_management.h"
#include "debug.h"

namespace fk {

constexpr uint32_t Interval = 5000;
constexpr const char Log[] = "Power";

void Power::setup() {
    Wire.begin();

    gauge.powerOn();
}

void Power::tick() {
    if (millis() > time) {
        time = millis() + Interval;
        auto stateOfCharge = gauge.stateOfCharge();
        auto voltage = gauge.cellVoltage();
        gauge.version();
        debugfpln(Log, "Tick (%.2f%% / %.2fmv)", stateOfCharge, voltage);
    }
}

}
