#include "live_data_reading.h"
#include "live_data.h"
#include "gather_readings.h"

namespace fk {

void LiveDataReading::task() {
    #if !defined(FK_NATURALIST)
    GatherReadings gatherReadings{
        1,
        *services().state,
        *services().leds,
        *services().moduleCommunications
    };

    gatherReadings.enqueued();

    while (simple_task_run(gatherReadings)) {
        services().alive();
        services().moduleCommunications->task();
    }

    services().liveData->completed();

    back();
    #else
    auto state = services().states->readings.next_state_ptr;

    resume_at_back();

    state->entry();

    while (true) {
        state->task();

        if (transitioned()) {
            break;
        }
    }

    #endif
}

}
