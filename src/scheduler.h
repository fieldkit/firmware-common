#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "rtc.h"

namespace fk {

struct TimeSpec {
    int8_t fixed;
    int8_t interval;
};

class SchedulerTask {
public:
};

class PeriodicTask : public SchedulerTask {
private:
    uint32_t interval{ 0 };
    uint32_t lastRan{ 0 };
    Task *task;

public:
    PeriodicTask(uint32_t interval, Task &task) : interval(interval), task(&task) {
    }

public:
    bool shouldRun();
    Task &getTask() {
        return *task;
    }

};

class ScheduledTask : public SchedulerTask {
private:
    TimeSpec second;
    TimeSpec minute;
    TimeSpec hour;
    TimeSpec day;
    DateTime lastRan;
    Task *task;

public:
    ScheduledTask(TimeSpec second, TimeSpec minute, TimeSpec hour, TimeSpec day, Task &task) :
        second(second), minute(minute), hour(hour), day(day), task(&task) {
    }

public:
    TimeSpec &getSecond() { return second; }
    TimeSpec &getMinute() { return minute; }
    TimeSpec &getHour() { return hour; }
    TimeSpec &getDay() { return day; }

    void setSecond(TimeSpec spec) { second = spec; }
    void setMinute(TimeSpec spec) { minute = spec; }
    void setHour(TimeSpec spec) { hour = spec; }
    void setDay(TimeSpec spec) { day = spec; }

public:
    bool shouldRun(DateTime now);

    bool valid();

    bool matches(DateTime now);

    uint32_t getNextRunTime(DateTime &after);

    Task &getTask() {
        return *task;
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
    CoreState *state;
    ClockType *clock;
    TaskQueue *queue;
    ScheduledTask *tasks;
    PeriodicTask *periodic;
    uint32_t lastCheckAt{ 0 };
    uint32_t lastStatusAt{ 0 };
    size_t numberOfTasks;
    size_t numberOfPeriodics;

public:
    template<size_t N, size_t M>
    Scheduler(CoreState &state, ClockType &clock, TaskQueue &queue, ScheduledTask (&tasks)[N], PeriodicTask (&periodic)[M]) :
        Task("Scheduler"), state(&state), clock(&clock), queue(&queue), tasks(tasks), periodic(periodic), numberOfTasks(N), numberOfPeriodics(M) {
    }

    template<size_t M>
    Scheduler(CoreState &state, ClockType &clock, TaskQueue &queue, PeriodicTask (&periodic)[M]) :
        Task("Scheduler"), state(&state), clock(&clock), queue(&queue), tasks(nullptr), periodic(periodic), numberOfTasks(0), numberOfPeriodics(M) {
    }

public:
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
