#include "take_readings.h"
#include "gather_readings.h"
#include "reboot_device.h"
#include "file_system.h"

namespace fk {

void TakeReadings::entry() {
    MainServicesState::entry();
    remaining_ = services().state->readingsToTake();
}

void TakeReadings::task() {
    if (!services().state->hasSensorModules()) {
        log("No sensor modules, skipping.");
        resume();
        return;
    }

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
        TwoWireStatistics tws;

        while (runner.run()) {
            services().alive();
            services().moduleCommunications->task(tws);
        }

        if (runner.error()) {
            // Were we able to talk to the module at all?
            if (!tws.any_responses()) {
                log("No valid responses from the module.");
                transit<RebootDevice>();
                return;
            }
            break;
        }

        remaining_--;
    }

    services().fileSystem->flush();

    resume();
}

}
