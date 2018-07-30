#include "scan_attached_deviecs.h"
#include "reboot_device.h"
#include "wifi_startup.h"

namespace fk {

class NoModulesThrottle : public MainServicesState {
private:
    uint32_t entered_{ 0 };

public:
    const char *name() const override {
        return "NoModulesThrottle";
    }

public:
    void entry() override {
        MainServicesState::entry();
        entered_ = fk_uptime();
    }

    void task() override {
        services().alive();

        if (fk_uptime() - entered_ > NoModulesRebootWait) {
            transit<RebootDevice>();
        }
    }
};

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

    #ifdef FK_CORE_REQUIRE_MODULES
    if (services().state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
        log("No attached modules.");
        transit<NoModulesThrottle>();
        return;
    }
    #endif

    #endif

    transit<WifiStartup>();
}

}
