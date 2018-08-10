#include "copy_module_data.h"
#include "watchdog.h"
#include "transmissions.h"
#include "performance.h"
#include "flash_reader.h"
#include "checksum_streams.h"

namespace fk {

void CopyModuleData::task() {
    SerialFlashChip flash;

    if (!flash.begin(Hardware::FLASH_PIN_CS)) {
        log("Error opening serial flash");
        back();
        return;
    }

    FlashReader reader(&flash, 0, 256 * 1024);
    Crc32Reader crc32{ reader };

    PrepareTransmissionData prepareTransmissionData{
        *services().state,
        *services().moduleCommunications,
        &crc32
    };

    prepareTransmissionData.enqueued();

    while (simple_task_run(prepareTransmissionData)) {
        services().watchdog->task();
        services().moduleCommunications->task();
    }

    log("Crc32: %lu", crc32.checksum());

    back();
}

}
