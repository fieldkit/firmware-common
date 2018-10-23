#include "scan_attached_devices.h"
#include "configuration.h"
#include "reboot_device.h"
#include "wifi_startup.h"
#include "no_modules.h"
#include "idle.h"

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

    if (first_) {
        first_ = false;
        transit<WifiStartup>();
        return;
    }
    else {
        if (!services().state->hasSensorModules()) {
            if (configuration.no_modules_sleep > 0) {
                transit<NoModules>();
                return;
            }
        }

        transit<Idle>();
        return;
    }

    #endif

    transit<WifiStartup>();
}

}
