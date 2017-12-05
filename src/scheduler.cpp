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
    return spec.fixed > -1 || spec.divisor > -1;
}

static bool matches(const TimeSpec &spec, int8_t value) {
    if (spec.fixed > -1) {
        return value == spec.fixed;
    }
    if (spec.divisor > -1) {
        return (value % spec.divisor) == 0;
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
