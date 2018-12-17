#include "wifi_states.h"
#include "core_state.h"
#include "app_servicer.h"
#include "wifi_server.h"
#include "discovery.h"
#include "live_data.h"
#include "live_data_reading.h"

namespace fk {

void WifiState::serve() {
    services().ntp.task();

    services().state->updateIp(WiFi.localIP());

    // Before Scheduler so we service before transitioning to scheduled states.
    if (services().server->listen()) {
        transit(services().appServicer);
        return;
    }

    services().alive();
    services().discovery->task();
    services().scheduledTasks();

    if (services().liveData->is_ready_for_reading()) {
        transit<LiveDataReading>();
    }
}

}
