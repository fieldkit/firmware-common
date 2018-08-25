#include "upgrade_module_firmware.h"
#include "watchdog.h"
#include "transmissions.h"
#include "firmware_storage.h"
#include "idle.h"

namespace fk {

void UpgradeModuleFirmware::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    firmware_header_t header;
    if (!firmwareStorage.header(FirmwareBank::ModuleNew, header)) {
        transit_into<Idle>();
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

    transit_into<Idle>();
}

}
