#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

namespace fk {

class ScheduledTask {
private:
    int8_t second;
    int8_t minute;
    int8_t hour;
    int8_t day;
    bool triggered;
    Task *task;

public:
    ScheduledTask(int8_t second, int8_t minute, int8_t hour, int8_t day, Task &task) :
        second(second), minute(minute), hour(hour), day(day), task(&task) {
    }

    bool shouldFire() {
        return false;
    };

};

class Scheduler : public ActiveObject {
private:
    CoreState *state;
    ScheduledTask *tasks;
    size_t numberOfTasks;

public:
    template<size_t N>
    Scheduler(CoreState &state, ScheduledTask (&tasks)[N]) : ActiveObject("Scheduler"), state(&state), tasks(tasks), numberOfTasks(N) {
    }

};

}

#endif
