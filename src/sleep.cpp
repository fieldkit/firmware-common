#include "sleep.h"

namespace fk {

void Sleep::entry() {
    MainServicesState::entry();
    activity_ = 0;

    FormattedTime wakeFormatted{ { clock.getTime() + maximum_ } };
    log("Sleeping for %lu (%s)", maximum_, wakeFormatted.toString());
}

void Sleep::react(UserButtonEvent const &ignored) {
    activity_ = fk_uptime();
}

void Sleep::task() {
    auto started = fk_uptime();
    auto stopping = started + (maximum_ * 1000);

    services().fileSystem->flush();

    while (fk_uptime() < stopping) {
        auto delayed = false;

        if (activity_ == 0 || fk_uptime() - activity_ > 10000) {
            if (!fk_console_attached()) {
                auto left = stopping - fk_uptime();
                if (left > SleepMaximumGranularity) {
                    services().watchdog->sleep(SleepMaximumGranularity);
                    delayed = true;
                }
            }

            if (!delayed) {
                delay(1000);
            }
        }
        else {
            delay(10);
        }

        services().alive();

        // Should never be scheduled events during this.
        if (transitioned()) {
            return;
        }
    }

    back();
}

}
