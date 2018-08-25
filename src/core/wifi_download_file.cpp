#include "wifi_download_file.h"
#include "download_file_task.h"
#include "app_servicer.h"
#include "wifi_connection_completed.h"

namespace fk {

void WifiDownloadFile::task() {
    StaticPool<384> pool{"WifiDownloadFile"};
    AppReplyMessage reply(&pool);

    DownloadFileTask task{
        *services().fileSystem,
        *services().state,
        reply,
        connection_->getBuffer(),
        *connection_,
        settings_
    };

    task.enqueued();

    while (simple_task_run(task)) {
        services().leds->task();
        services().watchdog->task();
    }

    services().appServicer->flushAndClose();

    transit<WifiConnectionCompleted>();
}

}
