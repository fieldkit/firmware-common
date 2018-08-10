#include "copy_module_data.h"
#include "watchdog.h"
#include "transmissions.h"
#include "performance.h"

namespace fk {

void CopyModuleData::task() {
    FileCopySettings fileCopySettings{ FileNumber::Data };

    PrepareTransmissionData prepareTransmissionData{
        *services().state,
        *services().fileSystem,
        *services().moduleCommunications,
        fileCopySettings,
    };

    prepareTransmissionData.enqueued();

    while (simple_task_run(prepareTransmissionData)) {
        services().alive();
        services().moduleCommunications->task();
    }

    back();
}

}
