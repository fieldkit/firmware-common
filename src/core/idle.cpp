#include "idle.h"
#include "scheduler.h"
#include "low_power_sleep.h"
#include "sleep.h"
#include "reboot_device.h"

namespace fk {

void Idle::entry() {
    MainServicesState::entry();

    services().watchdog->idling();

    auto now = clock.now();
    status(now, services().scheduler->nextTask());

    entered_ = now.unixtime();
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

void Idle::status(DateTime now, lwcron::Scheduler::TaskAndTime nextTask) {
    FormattedTime nowFormatted{ now };
    FormattedTime runsAgainFormatted{ { nextTask.time } };
    auto prettyTask = nextTask.task != nullptr ? nextTask.task->toString() : "<none>";
    log("Status (now = %s, %lu) (task '%s' = %s, %lu)", nowFormatted.toString(), now.unixtime(), prettyTask, runsAgainFormatted.toString(), nextTask.time);

    TaskLogger logger;
    services().scheduler->accept(logger);
}

void Idle::task() {
    services().alive();

    // NOTE: The assertions and the logging below are to find and fix
    // https://code.conservify.org/jira/browse/FK-434 and can be removed once
    // we're satisfied that the issue is fixed.

    if (fk_uptime() - checked_ > SchedulerCheckInterval) {
        auto now = services().scheduledTasks();
        auto nowUnix = now.unixtime();
        if (!(nowUnix >= entered_)) {
            log("FATAL: entered_ = %lu, nowUnix = %lu", entered_, nowUnix);
            fk_assert(nowUnix >= entered_);
        }
        if (transitioned()) {
            return;
        }

        auto nextTask = services().scheduler->nextTask();
        if (nextTask) {
            if (!(nextTask.time >= nowUnix) || !(nextTask.time >= entered_)) {
                log("FATAL: entered_ = %lu, nowUnix = %lu, nextTask.time = %lu", entered_, nowUnix, nextTask.time);
                fk_assert(nextTask.time >= nowUnix);
                fk_assert(nextTask.time >= entered_);
            }

            auto seconds = nextTask.time - nowUnix;
            if (seconds > SleepMinimumSeconds) {
                auto sleepingFor = seconds - SleepLeadingWakeupSeconds;
                log("Sleeping: %lu - %lu = %lu (%lu)", nextTask.time, nowUnix, seconds, sleepingFor);
                status(now, nextTask);
                transit_into<Sleep>(sleepingFor);
                return;
            }
        }
        else {
            error("No next task!");
            transit_into<Sleep>((uint32_t)300);
            return;
        }

        checked_ = fk_uptime();
    }

    if (services().fileSystem->files().errors()) {
        transit_into<RebootDevice>();
        return;
    }
}

}
