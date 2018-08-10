#include "copy_module_data.h"
#include "watchdog.h"
#include "transmissions.h"
#include "performance.h"
#include "flash_reader.h"

namespace fk {

void CopyModuleData::task() {
    SerialFlashChip flash;

    if (!flash.begin(Hardware::FLASH_PIN_CS)) {
        log("Error opening serial flash");
        back();
        return;
    }

    FlashReader reader(&flash, 0, 256 * 1024);

    PrepareTransmissionData prepareTransmissionData{
        *services().state,
        *services().moduleCommunications,
        &reader
    };

    prepareTransmissionData.enqueued();

    while (simple_task_run(prepareTransmissionData)) {
        services().watchdog->task();
        services().moduleCommunications->task();
    }

    back();
}

}
