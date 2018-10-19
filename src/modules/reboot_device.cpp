#include "reboot_device.h"

namespace fk {

void RebootDevice::task() {
    log("Rebooting!");

    fk_system_reset();
}

}
