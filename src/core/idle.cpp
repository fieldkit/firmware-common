#include "idle.h"
#include "low_power_sleep.h"
#include "scheduler.h"

namespace fk {

void Idle::entry() {
    MainServicesState::entry();

    services().watchdog->idling();

    status(clock.now(), services().scheduler->nextTask());
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

class TaskLogger : public lwcron::TaskVisitor {
public:
    void visit(lwcron::PeriodicTask &task) override {
        sdebug() << "PeriodicTask<'" << task.toString() << "' every " << task.interval() << "s" << (task.valid() ? "" : " INVALID") << ">" << endl;
    }

    void visit(lwcron::CronTask &task) override {
        sdebug() << "CronTask<'" << task.toString() << "'" << (task.valid() ? "" : " INVALID") << ">" << endl;
    }
};

void Idle::status(DateTime now, lwcron::Scheduler::TaskAndTime nextTask) {
    FormattedTime nowFormatted{ now };
    FormattedTime runsAgainFormatted{ { nextTask.time } };
    auto prettyTask = nextTask.task != nullptr ? nextTask.task->toString() : "<none>";
    log("Status (now = %s) (task '%s' = %s)", nowFormatted.toString(), prettyTask, runsAgainFormatted.toString());

    TaskLogger logger;
    services().scheduler->accept(logger);
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
}

}
