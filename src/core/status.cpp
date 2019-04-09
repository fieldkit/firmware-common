#include "status.h"
#include "core_fsm.h"
#include "tuning.h"
#include "wifi_tools.h"
#include "device_id.h"

namespace fk {

constexpr uint32_t PersistedLogInterval = 60 * 1000;

TaskEval Status::task() {
    if (fk_uptime() - last_tick_ > StatusInterval) {
        auto now = clock.now();
        auto battery = state_->getStatus().battery;
        auto state = CoreFsm::current().name();
        auto ipAddress = state_->getStatus().ip;
        IpAddress4 ip{ ipAddress };
        FormattedTime nowFormatted{ now };
        const char *wifi = (ipAddress == 0) ? "None" : getWifiStatus();

        auto level = LogLevels::TRACE;
        if (fk_uptime() - last_logged_ > PersistedLogInterval) {
            level = LogLevels::INFO;
            last_logged_ = fk_uptime();
        }

        alogf(level, "Status", "%s (%" PRIu32 ") (%.2f%% / %.2fmv, %.3fmAh ± %.3fmAh, I = %0.3fma, %s) (%" PRIu32 " free) (%s) (%s) (%s) (%s)",
              nowFormatted.toString(), now.unixtime(),
              battery.percentage, battery.voltage, battery.coulombs, battery.delta, battery.ma, battery.charging() ? "CHG" : "DIS",
              fk_free_memory(), deviceId.toString(), ip.toString(), state, wifi);

        last_tick_ = fk_uptime();

        leds_->notifyBattery(battery.percentage);
    }
    return TaskEval::idle();
}

}
