#include "copy_module_data.h"
#include "watchdog.h"
#include "transmissions.h"
#include "firmware_storage.h"
#include "check_power.h"

namespace fk {

void CopyModuleData::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    firmware_header_t header;
    if (!firmwareStorage.header(FirmwareBank::ModuleNew, header)) {
        transit_into<CheckPower>();
        return;
    }

    auto reader = firmwareStorage.read(FirmwareBank::ModuleNew);

    ModuleCopySettings settings = {
        FirmwareBank::ModuleNew,
        reader->size(),
        header.etag
    };

    PrepareTransmissionData prepareTransmissionData{
        *services().state,
        *services().moduleCommunications,
        reader,
        settings
    };

    prepareTransmissionData.enqueued();

    while (simple_task_run(prepareTransmissionData)) {
        services().watchdog->task();
        services().moduleCommunications->task();
    }

    transit_into<CheckPower>();
}

}
