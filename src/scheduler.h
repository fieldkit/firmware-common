#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "rtc.h"
#include "core_fsm.h"

namespace fk {

struct TimeSpec {
    int8_t fixed;
    int8_t interval;
};

class SchedulerTask {
};

class PeriodicTask : public SchedulerTask {
private:
    uint32_t interval_{ 0 };
    uint32_t lastRanTick_{ 0 };
    uint32_t lastRanTime_{ 0 };
    SchedulerEvent event_;

public:
    uint32_t interval() const {
        return interval_;
    }

public:
    PeriodicTask(uint32_t interval, SchedulerEvent event) : interval_(interval), event_(event) {
    }

public:
    bool shouldRun(DateTime now);

    bool valid() {
        return interval_ > 0;
    }

    uint32_t getNextRunTime(DateTime &after);

    SchedulerEvent &event() {
        return event_;
    }

};

class ScheduledTask : public SchedulerTask {
private:
    TimeSpec second_;
    TimeSpec minute_;
    TimeSpec hour_;
    TimeSpec day_;
    DateTime lastRan_;
    SchedulerEvent event_;

public:
    ScheduledTask(TimeSpec second, TimeSpec minute, TimeSpec hour, TimeSpec day, SchedulerEvent event) :
        second_(second), minute_(minute), hour_(hour), day_(day), event_(event) {
    }

public:
    TimeSpec &getSecond() { return second_; }
    TimeSpec &getMinute() { return minute_; }
    TimeSpec &getHour() { return hour_; }
    TimeSpec &getDay() { return day_; }

    void setSecond(TimeSpec spec) { second_ = spec; }
    void setMinute(TimeSpec spec) { minute_ = spec; }
    void setHour(TimeSpec spec) { hour_ = spec; }
    void setDay(TimeSpec spec) { day_ = spec; }

public:
    bool shouldRun(DateTime now);

    bool valid();

    bool matches(DateTime now);

    uint32_t getNextRunTime(DateTime &after);

    SchedulerEvent &event() {
        return event_;
    }

};

enum class ScheduleKind {
    Readings = 0,
    Transmission,
    Status,
    Location,
    NumberOfMandatorySchedules,
};

class Scheduler : public Task {
    static constexpr uint32_t CheckInterval = 500;
    static constexpr uint32_t StatusInterval = 1000;

private:
    ClockType *clock;
    ScheduledTask *tasks;
    PeriodicTask *periodic;
    uint32_t lastCheckAt{ 0 };
    size_t numberOfTasks;
    size_t numberOfPeriodics;

public:
    template<size_t N, size_t M>
    Scheduler(ClockType &clock, ScheduledTask (&tasks)[N], PeriodicTask (&periodic)[M]) :
        Task("Scheduler"), clock(&clock), tasks(tasks), periodic(periodic), numberOfTasks(N), numberOfPeriodics(M) {
    }

    template<size_t M>
    Scheduler(ClockType &clock, PeriodicTask (&periodic)[M]) :
        Task("Scheduler"), clock(&clock), tasks(nullptr), periodic(periodic), numberOfTasks(0), numberOfPeriodics(M) {
    }

public:
    void started();

    TaskEval task() override;

public:
    ScheduledTask &getTaskSchedule(ScheduleKind kind);
    uint32_t getNextTaskTime(DateTime &after);
    uint32_t getNextTaskTime();

    struct NextTaskInfo {
        uint32_t now;
        uint32_t time;
        uint32_t seconds;
    };

    NextTaskInfo getNextTask();

};

}

#endif
