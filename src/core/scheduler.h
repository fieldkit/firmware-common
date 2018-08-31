#ifndef FK_SCHEDULER_H_INCLUDED
#define FK_SCHEDULER_H_INCLUDED

#include <lwcron/lwcron.h>

#include "core_fsm.h"

namespace fk {

class SchedulerTask {
public:
    virtual void write(ostreamtype &os) const = 0;

public:
    friend ostreamtype& operator<<(ostreamtype& os, const SchedulerTask &task) {
        task.write(os);
        return os;
    }

};

class PeriodicTask : public lwcron::PeriodicTask, public SchedulerTask {
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

public:
    const char *toString() const override {
        return event_.toString();
    }

    void write(ostreamtype &os) const override {
        os << "PeriodicTask<'" << toString() << "' " << interval() << (valid() ? "" : " INVALID") << ">";
    }

};

class CronTask : public lwcron::CronTask, public SchedulerTask {
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

public:
    const char *toString() const override {
        return event_.toString();
    }

    void write(ostreamtype &os) const override {
        os << "CronTask<'" << toString() << "'" << (valid() ? "" : " INVALID") << ">";
    }

};

}

#endif
