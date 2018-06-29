#include "status.h"
#include "tuning.h"

namespace fk {

TaskEval Status::task() {
    if (millis() - lastTick > StatusInterval) {
        IpAddress4 ip{ state->getStatus().ip };
        auto now = clock.now();
        auto percentage = state->getStatus().batteryPercentage;
        loginfof("Status", "Status %" PRIu32 " (%.2f%% / %.2fmv) (%" PRIu32 " free) (%s) (%s) busy=%d reading=%d", now.unixtime(),
                 percentage, state->getStatus().batteryVoltage,
                 fk_free_memory(), ip.toString(), deviceId.toString(),
                 state->isBusy(), state->isReadingInProgress());
        lastTick = millis();

        if (percentage < StatusBatteryBlinkThreshold) {
            auto batteryBlinks = (uint8_t)(percentage / 10.0f);
            leds->status(batteryBlinks);
        }
    }
    return TaskEval::idle();
}

}
