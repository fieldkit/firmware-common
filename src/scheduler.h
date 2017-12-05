#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "rtc.h"

namespace fk {

struct TimeSpec {
    int8_t fixed;
    int8_t divisor;
};

class ScheduledTask {
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
    bool shouldRun(DateTime now);

    bool valid();

    bool matches(DateTime now);

    Task &getTask() {
        return *task;
    }

};

class Scheduler : public ActiveObject {
private:
    CoreState *state;
    Clock *clock;
    ScheduledTask *tasks;
    size_t numberOfTasks;

public:
    template<size_t N>
    Scheduler(CoreState &state, Clock &clock, ScheduledTask (&tasks)[N]) :
        ActiveObject("Scheduler"), state(&state), clock(&clock), tasks(tasks), numberOfTasks(N) {
    }

public:
    void idle() override;

};

}

#endif
