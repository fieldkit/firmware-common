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
