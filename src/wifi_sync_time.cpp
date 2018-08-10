#include "wifi_sync_time.h"
#include "simple_ntp.h"
#include "check_firmware.h"

namespace fk {

void WifiSyncTime::task() {
    if (!success_) {
        SimpleNTP ntp(clock);

        ntp.enqueued();

        while (elapsed() < NtpMaximumWait) {
            services().watchdog->task();
            if (!simple_task_run(ntp)) {
                success_ = true;
                break;
            }
        }
    }

    transit<CheckAllAttachedFirmware>();
}

}
