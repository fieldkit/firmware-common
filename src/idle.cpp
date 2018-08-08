#include "idle.h"
#include "low_power_sleep.h"

namespace fk {

void Idle::entry() {
    MainServicesState::entry();

    auto now = clock.now();
    auto nextTask = services().scheduler->nextTask();
    FormattedTime nowFormatted{ now };
    FormattedTime runsAgainFormatted{ { nextTask.time } };
    log("Idling (now = %s) (again = %s)", nowFormatted.toString(), runsAgainFormatted.toString());
}

void Idle::react(LowPowerEvent const &lpe) {
    if (fk_console_attached()) {
        log("Console attached, ignoring LowPowerEvent.");
    }
    else {
        transit<LowPowerSleep>();
    }
}

void Idle::react(SchedulerEvent const &se) {
    if (se.deferred) {
        warn("Scheduler Event!");
        transit(se.deferred);
    }
}

void Idle::task() {
    services().alive();

    if (fk_uptime() - checked_ > SchedulerCheckInterval) {
        auto now = services().scheduledTasks();
        if (transitioned()) {
            return;
        }

        auto nextTask = services().scheduler->nextTask();
        if (nextTask) {
            auto seconds = nextTask.time - now.unixtime();
            if (seconds > SleepMinimumSeconds) {
                auto sleepingFor = seconds - SleepLeadingWakeupSeconds;
                FormattedTime nowFormatted{ now };
                FormattedTime runsAgainFormatted{ { nextTask.time } };
                log("Sleeping (now = %s) (again = %s) (%lu)", nowFormatted.toString(), runsAgainFormatted.toString(), sleepingFor);
                transit_into<Sleep>(sleepingFor);
                return;
            }
        }
        else {
            log("No next task. Possible configuration/firmware issue?");
            transit_into<Sleep>(300);
            return;
        }

        checked_ = fk_uptime();
    }
}

}
