#include "reboot_device.h"
#include "file_system.h"
#include "initialized.h"

namespace fk {

void RebootDevice::task() {
    log("Rebooting!");

    services().fileSystem->flush();

    if (fk_console_attached()) {
        transit<Initialized>();
    }
    else {
        NVIC_SystemReset();
    }
}

}
