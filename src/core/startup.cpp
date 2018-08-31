#include <lwcron/lwcron.h>

#include "startup.h"
#include "core_state.h"
#include "hardware.h"
#include "device_id.h"
#include "rtc.h"
#include "tuning.h"
#include "debug.h"
#include "file_system.h"
#include "serial_number.h"
#include "radio_service.h"

namespace fk {

StartSystem::Deferred StartSystem::configure_;

void StartSystem::task() {
    clock.begin();

    // TODO: Maybe write this to memory just in case this fails in the future?
    fk_assert(deviceId.initialize(*services().bus));
    services().state->setDeviceId(deviceId.toString());

    Hardware::cyclePeripherals();

    // TODO: Move this after low power sleep. Also going to need to work around
    // the startup delay.
    Hardware::cycleModules();

    #ifdef FK_ENABLE_FLASH
    setupFlash();
    #else
    log("Flash memory disabled");
    #endif

    #ifdef FK_ENABLE_RADIO
    if (!services().radio->setup(deviceId)) {
        log("Radio service unavailable");
    }
    else {
        log("Radio service ready");
    }
    #else
    log("Radio service disabled");
    #endif

    fk_assert(services().fileSystem->setup());

    SerialNumber serialNumber;
    log("Serial(%s)", serialNumber.toString());
    log("DeviceId(%s)", deviceId.toString());
    log("Hash(%s)", firmware_version_get());
    log("Build(%s)", firmware_build_get());
    log("API(%s)", WifiApiUrlIngestionStream);

    auto now = clock.now();
    FormattedTime nowFormatted{ now };
    log("Now: %s (%lu)", nowFormatted.toString(), now.unixtime());

    services().watchdog->started();
    services().state->started();
    services().scheduler->begin(lwcron::DateTime{ now.unixtime() });

    transit(configure_);
}

void StartSystem::setupFlash() {
    auto flashFs = services().flashFs;
    auto flashState = services().flashState;

    if (!flashFs->initialize(Hardware::FLASH_PIN_CS)) {
        log("Flash unavailable.");
        fk_assert(false);
    }

    if (!flashState->initialize()) {
        log("Flash unavailable.");
        fk_assert(false);
    }

    if (!flashFs->reclaim(*flashState)) {
        log("Flash reclaim failed.");
        fk_assert(false);
    }
}

}
