#include "scan_attached_devices.h"
#include "reboot_device.h"
#include "wifi_startup.h"

namespace fk {

void ScanAttachedDevices::task() {
    #if !defined(FK_NATURALIST)

    uint8_t addresses[4]{ 7, 8, 9, 0 };

    AttachedDevices attachedDevices{
        *services().state,
        *services().leds,
        *services().moduleCommunications,
        addresses,
    };

    while (simple_task_run(attachedDevices)) {
        // TODO: Should never take so long we need the watchdog.
        services().leds->task();
        services().moduleCommunications->task();
    }

    #endif

    transit<WifiStartup>();
}

}
