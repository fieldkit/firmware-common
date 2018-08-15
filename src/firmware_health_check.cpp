#include "firmware_health_check.h"
#include "scan_attached_devices.h"
#include "firmware_storage.h"

extern uint32_t __exidx_end;
extern uint32_t __end__;
extern uint32_t __data_end__;
extern uint32_t __etext;

namespace fk {

void FirmwareHealthCheck::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    firmware_header_t header;
    if (firmwareStorage.header(FirmwareBank::CoreGood, header)) {
        if (header.version != FIRMWARE_VERSION_INVALID) {
            log("Have: '%s' (%lu bytes)", header.etag, header.size);
            transit<ScanAttachedDevices>();
            return;
        }
    }

    auto writer = firmwareStorage.write();
    header.version = 1;
    header.time = clock.getTime();
    header.size = 256 * 1024 - 2048;
    header.etag[0] = 0;

    auto headerBytes = writer->write((uint8_t *)&header, sizeof(firmware_header_t));
    if (headerBytes != sizeof(firmware_header_t)) {
        error("Writing header failed.");
    }

    log("Saving existing firmware (%p)", (void *)FIRMWARE_NVM_PROGRAM_ADDRESS);

    uint32_t bytes = 0;
    uint8_t *ptr = (uint8_t *)FIRMWARE_NVM_PROGRAM_ADDRESS;

    while (bytes < header.size) {
        auto written = writer->write(ptr, 1024);
        if (written == 0) {
            break;
        }
        bytes += written;
    }

    if (bytes == header.size) {
        log("Done, filling bank.");
        if (!firmwareStorage.update(FirmwareBank::CoreGood, writer, "")) {
            error("Error");
        }
    }

    transit<ScanAttachedDevices>();
}

}
