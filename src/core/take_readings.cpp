#include "take_readings.h"
#include "gather_readings.h"
#include "file_system.h"

namespace fk {

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

        while (simple_task_run(gatherReadings)) {
            services().alive();
            services().moduleCommunications->task();
        }

        remaining_--;
    }

    services().fileSystem->flush();

    resume();
}

}
