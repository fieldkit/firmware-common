#include "wifi_query_module.h"
#include "app_module_query_task.h"
#include "app_servicer.h"
#include "wifi_connection_completed.h"

namespace fk {

void WifiQueryModule::task() {
    fk_assert(reply_ != nullptr);
    fk_assert(query_ != nullptr);

    StaticPool<384> pool{"WifiQueryModule"};
    AppReplyMessage reply(&pool);

    AppModuleQueryTask task{
        *reply_,
        *query_,
        connection_->getBuffer(),
        8,
        *services().moduleCommunications
    };

    task.enqueued();

    auto started = fk_uptime();

    while (simple_task_run(task)) {
        if (fk_uptime() - started > 10000) {
            break;
        }

        services().leds->task();
        services().watchdog->task();
        services().moduleCommunications->task();
    }

    services().appServicer->flushAndClose();

    transit<WifiConnectionCompleted>();
}

}
