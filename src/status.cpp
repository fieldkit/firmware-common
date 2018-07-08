#include "status.h"
#include "core_fsm.h"
#include "tuning.h"

namespace fk {

TaskEval Status::task() {
    if (fk_uptime() - lastTick > StatusInterval) {
        IpAddress4 ip{ state->getStatus().ip };
        auto now = clock.now();
        auto percentage = state->getStatus().batteryPercentage;
        auto voltage = state->getStatus().batteryVoltage;
        auto state = CoreFsm::current().name();
        FormattedTime nowFormatted{ now };

        loginfof("Status", "%s (%" PRIu32 ") (%.2f%% / %.2fmv) (%" PRIu32 " free) (%s) (%s) (%s)",
                 nowFormatted.toString(), now.unixtime(),
                 percentage, voltage, fk_free_memory(),
                 deviceId.toString(), ip.toString(), state);

        lastTick = fk_uptime();

        if (percentage < BatteryStatusBlinkThreshold) {
            auto batteryBlinks = (uint8_t)(percentage / 10.0f);
            leds->status(batteryBlinks);
        }
    }
    return TaskEval::idle();
}

}
