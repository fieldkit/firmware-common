#include "scheduler.h"

namespace fk {

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

bool PeriodicTask::shouldRun() {
    if (millis() - lastRan > interval) {
        lastRan = millis();
        return true;
    }
    return false;
}

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

uint32_t ScheduledTask::getNextRunTime(DateTime &after) {
    if (!valid()) {
        return UINT32_MAX;
    }
    DateTime copy(after);
    if (fk::valid(second)) {
        if (second.fixed >= 0) {
            auto r = second.fixed - after.second();
            if (r <= 0) {
                r += 60;
            }
            copy = copy + TimeSpan{ r };
        } else if (second.interval >= 0) {
            auto r = second.interval - (copy.second() % second.interval);
            copy = copy + TimeSpan{ r };
        }
    }
    if (fk::valid(minute)) {
        if (minute.fixed >= 0) {
            auto r = minute.fixed - after.minute();
            if (r <= 0) {
                r += 60;
            }
            copy = copy + TimeSpan{ r * 60 };
        } else if (minute.interval >= 0) {
            auto r = minute.interval - (copy.minute() % minute.interval);
            copy = copy + TimeSpan{ r * 60 };
        }
    }
    return copy.unixtime();
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
                DateTime runsAgain{tasks[i]. getNextRunTime(now) };
                FormattedTime nowFormatted{ now };
                FormattedTime runsAgainFormatted{ runsAgain };
                log("%s: run task (again = %s)", nowFormatted.toString(), runsAgainFormatted.toString());
                push(task);
            }
        }
    }
    for (size_t i = 0; i < numberOfPeriodics; ++i) {
        if (periodic[i].shouldRun()) {
            auto &task = periodic[i].getTask();
            push(task);
        }
    }
}

ScheduledTask &Scheduler::getTaskSchedule(ScheduleKind kind) {
    return tasks[(size_t)kind];
}

uint32_t Scheduler::getNextTaskTime() {
    auto now = clock->now();
    return getNextTaskTime(now);
}

uint32_t Scheduler::getNextTaskTime(DateTime &after) {
    auto winner = UINT32_MAX;
    for (size_t i = 0; i < numberOfTasks; ++i) {
        auto next = tasks[i].getNextRunTime(after);
        if (next < winner) {
            winner = next;
        }
    }
    return winner;
}

}
