#include "wifi_live_data.h"
#include "wifi_connection_completed.h"
#include "gather_readings.h"

namespace fk {

void WifiLiveData::react(LiveDataEvent const &lde) {
    interval_ = lde.interval;
}

void WifiLiveData::react(AppQueryEvent const &aqe) {
    if (aqe.type == fk_app_QueryType_QUERY_LIVE_DATA_POLL) {
        lastPolled_ = fk_uptime();
    }
}

void WifiLiveData::entry() {
    WifiState::entry();

    lastReadings_ = 0;

    if (services().state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
        log("No attached modules.");
        transit<WifiConnectionCompleted>();
        return;
    }
}

void WifiLiveData::task() {
    if (interval_ == 0) {
        log("Cancelled");
        back();
        return;
    }

    if (fk_uptime() - lastPolled_ > LivePollInactivity) {
        log("Stopped due to inactivity.");
        transit<WifiConnectionCompleted>();
        return;
    }

    if (fk_uptime() - lastReadings_ > interval_) {
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
            serve();
        }

        log("Readings");
        lastReadings_ = fk_uptime();
    }

    serve();
}

}
