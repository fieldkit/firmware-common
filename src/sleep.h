#ifndef FK_SLEEP_H_INCLUDED
#define FK_SLEEP_H_INCLUDED

#include "core_fsm_states.h"
#include "file_system.h"

namespace fk {

class Sleep : public MainServicesState {
private:
    uint32_t maximum_{ 0 };
    uint32_t activity_{ 0 };

public:
    Sleep() {
    }

    Sleep(uint32_t maximum) : maximum_(maximum) {
    }

public:
    const char *name() const override {
        return "Sleep";
    }

public:
    void entry() override {
        MainServicesState::entry();
        activity_ = 0;

        FormattedTime wakeFormatted{ { clock.getTime() + maximum_ } };
        log("Sleeping for %lu (%s)", maximum_, wakeFormatted.toString());
    }

    void react(UserButtonEvent const &ignored) override {
        activity_ = fk_uptime();
    }

    void task() override {
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
};

}

#endif
