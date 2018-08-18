#include "module_firmware_self_flash.h"
#include "firmware_storage.h"

namespace fk {

void ModuleFirmwareSelfFlash::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    firmwareStorage.backup();

    log("Waiting for %lums", SelfFlashWaitPeriod);
    while (elapsed() < SelfFlashWaitPeriod) {
        services().alive();
    }

    firmware_self_flash();
}

}
