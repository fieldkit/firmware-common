#include "scheduler.h"

namespace fk {

bool ScheduledTask::shouldRun(DateTime now) {
    if (matches(now)) {
        if (lastRan.unixtime() == now.unixtime()) {
            return false;
        }
        lastRan = now;
        return true;
    }
    return false;
};

static bool valid(const TimeSpec &spec) {
    return spec.fixed > -1 || spec.interval > -1;
}

static bool matches(const TimeSpec &spec, int8_t value) {
    if (spec.fixed > -1) {
        return value == spec.fixed;
    }
    if (spec.interval > -1) {
        return (value % spec.interval) == 0;
    }
    return true;
}

bool ScheduledTask::valid() {
    return fk::valid(second) || fk::valid(minute);
}

bool ScheduledTask::matches(DateTime now) {
    if (!fk::matches(second, now.second())) {
        return false;
    }
    if (!fk::matches(minute, now.minute())) {
        return false;
    }
    if (!fk::matches(hour, now.hour())) {
        return false;
    }
    if (!fk::matches(day, now.day())) {
        return false;
    }

    return true;
}

void Scheduler::idle() {
    if (clock->isValid()) {
        auto now = clock->now();
        for (size_t i = 0; i < numberOfTasks; ++i) {
            if (tasks[i].valid() && tasks[i].shouldRun(now)) {
                auto &task = tasks[i].getTask();
                log("%s: run task", clock->nowString());
                push(task);
            }
        }
    }
}

ScheduledTask &Scheduler::getTaskSchedule(ScheduleKind kind) {
    return tasks[(size_t)kind];
}

}
