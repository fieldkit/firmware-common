#include "firmware_health_check.h"
#include "scan_attached_devices.h"

namespace fk {

void FirmwareHealthCheck::task() {
    transit<ScanAttachedDevices>();
}

}
