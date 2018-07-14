#include "scheduler.h"
#include "core_fsm.h"

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

bool PeriodicTask::shouldRun(DateTime now) {
    if (interval_ == 0) {
        return false;
    }
    if (fk_uptime() - lastRanTick_ > interval_) {
        lastRanTick_ = fk_uptime();
        lastRanTime_ = now.unixtime();
        return true;
    }
    return false;
}

uint32_t PeriodicTask::getNextRunTime(DateTime &after) {
    auto afterUnix = after.unixtime();
    auto intervalInSeconds = interval_ / 1000;
    if (lastRanTime_ == 0) {
        // NOTE: This isn't totally accurate, but it's good enough.
        return afterUnix + intervalInSeconds;
    }

    auto expected = lastRanTime_ + intervalInSeconds;
    if (expected < afterUnix) {
        return afterUnix;
    }

    return expected;
}

bool ScheduledTask::shouldRun(DateTime now) {
    if (matches(now)) {
        if (lastRan_.unixtime() == now.unixtime()) {
            return false;
        }
        lastRan_ = now;
        return true;
    }
    return false;
};

uint32_t ScheduledTask::getNextRunTime(DateTime &after) {
    if (!valid()) {
        return UINT32_MAX;
    }
    DateTime copy(after);
    if (fk::valid(second_)) {
        if (second_.fixed >= 0) {
            auto r = second_.fixed - after.second();
            if (r <= 0) {
                r += 60;
            }
            copy = copy + TimeSpan{ r };
        } else if (second_.interval >= 0) {
            auto r = second_.interval - (copy.second() % second_.interval);
            copy = copy + TimeSpan{ r };
        }
    }
    if (fk::valid(minute_)) {
        if (minute_.fixed >= 0) {
            auto r = minute_.fixed - after.minute();
            if (r <= 0) {
                r += 59;
            }
            copy = copy + TimeSpan{ r * 60 };
        } else if (minute_.interval >= 0) {
            auto r = minute_.interval - (copy.minute() % minute_.interval);
            copy = copy + TimeSpan{ r * 60 };
        }
    }
    return copy.unixtime();
}

bool ScheduledTask::valid() {
    return fk::valid(second_) || fk::valid(minute_);
}

bool ScheduledTask::matches(DateTime now) {
    if (!fk::matches(second_, now.second())) {
        return false;
    }
    if (!fk::matches(minute_, now.minute())) {
        return false;
    }
    if (!fk::matches(hour_, now.hour())) {
        return false;
    }
    if (!fk::matches(day_, now.day())) {
        return false;
    }

    return true;
}

void Scheduler::started() {
    if (numberOfPeriodics > 1) {
        for (size_t i = 0; i < numberOfPeriodics - 1; ++i) {
            for (size_t j = 0; j < numberOfPeriodics - 1 - i; ++j) {
                if (periodic[j].interval() < periodic[j + 1].interval()) {
                    auto temp = periodic[j];
                    periodic[j] = periodic[j + 1];
                    periodic[j + 1] = temp;
                }
            }
        }
    }

    for (size_t i = 0; i < numberOfPeriodics; ++i) {
        if (periodic[i].valid()) {
            log("Periodic: %lu -> %s", periodic[i].interval(), periodic[i].event().toString());
        }
    }

    for (size_t i = 0; i < numberOfTasks; ++i) {
        if (tasks[i].valid()) {
            log("Scheduled: %s", tasks[i].event().toString());
        }
    }
}

TaskEval Scheduler::task() {
    auto elapsed = fk_uptime() - lastCheckAt;
    if (elapsed < CheckInterval) {
        return TaskEval::idle();
    }

    auto now = clock->now();

    if (clock->isValid()) {
        lastCheckAt = fk_uptime();
        for (size_t i = 0; i < numberOfTasks; ++i) {
            if (tasks[i].valid() && tasks[i].shouldRun(now)) {
                auto &event = tasks[i].event();
                DateTime runsAgain{ tasks[i].getNextRunTime(now) };
                FormattedTime nowFormatted{ now };
                FormattedTime runsAgainFormatted{ runsAgain };
                #if FK_LOGGING_VERBOSITY > 2
                log("Run (now = %s) (again = %s)", nowFormatted.toString(), runsAgainFormatted.toString());
                #endif
                send_event(event);
                break;
            }
        }
    }
    for (size_t i = 0; i < numberOfPeriodics; ++i) {
        if (periodic[i].shouldRun(now)) {
            auto &event = periodic[i].event();
            #if FK_LOGGING_VERBOSITY > 2
            log("Run periodic");
            #endif
            send_event(event);
            break;
        }
    }

    return TaskEval::idle();
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
    for (size_t i = 0; i < numberOfPeriodics; ++i) {
        auto next = periodic[i].getNextRunTime(after);
        if (next < winner) {
            winner = next;
        }
    }
    return winner;
}

Scheduler::NextTaskInfo Scheduler::getNextTask() {
    auto now = clock->now();
    auto nextTask = getNextTaskTime(now);
    auto seconds = nextTask - now.unixtime();
    return { now.unixtime(), nextTask, seconds };
}

}
