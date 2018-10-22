#include "take_readings.h"
#include "gather_readings.h"
#include "file_system.h"

namespace fk {

class TaskRunner {
private:
    Task &task_;
    TaskEval eval_ = TaskEval::done();

public:
    TaskRunner(Task &task) : task_(task) {
    }

public:
    bool run() {
        eval_ = task_.task();
        if (eval_.isDone()) {
            task_.done();
            return false;
        }
        if (eval_.isError()) {
            task_.error();
            return false;
        }
        return true;
    }

    bool error() {
        return eval_.isError();
    }
};

void TakeReadings::entry() {
    MainServicesState::entry();
    remaining_ = services().state->readingsToTake();
}

void TakeReadings::task() {
    while (remaining_ > 0) {
        GatherReadings gatherReadings{
            remaining_,
            *services().state,
            *services().leds,
            *services().moduleCommunications
        };

        gatherReadings.enqueued();

        log("Taking reading %d", remaining_);

        TaskRunner runner{ gatherReadings };

        while (runner.run()) {
            services().alive();
            services().moduleCommunications->task();
        }

        if (runner.error()) {
            break;
        }

        remaining_--;
    }

    services().fileSystem->flush();

    resume();
}

}
