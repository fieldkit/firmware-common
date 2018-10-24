#include "firmware_health_check.h"
#include "scan_attached_devices.h"
#include "firmware_storage.h"

namespace fk {

void FirmwareSelfFlash::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    firmwareStorage.backup();

    log("Waiting for %lums", SelfFlashWaitPeriod);
    while (elapsed() < SelfFlashWaitPeriod) {
        services().alive();
    }

    firmware_self_flash();
}

void FirmwareHealthCheck::task() {
    transit_into<ScanAttachedDevices>();
}

}
