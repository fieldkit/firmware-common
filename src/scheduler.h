#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include <lwcron/lwcron.h>

#include "core_fsm.h"

namespace fk {

class PeriodicTask : public lwcron::PeriodicTask {
private:
    SchedulerEvent event_;

public:
    PeriodicTask(uint32_t interval, SchedulerEvent event) :
        lwcron::PeriodicTask(interval), event_(event) {
    }

public:
    void run() override {
        send_event(event_);
    }

};

class CronTask : public lwcron::CronTask {
private:
    SchedulerEvent event_;

public:
    CronTask(lwcron::CronSpec spec, SchedulerEvent event) :
        lwcron::CronTask(spec), event_(event) {
    }

public:
    void run() override {
        send_event(event_);
    }

};

}

#endif
