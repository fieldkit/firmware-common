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

void StartSystem::task() {
    clock.begin();

    // TODO: Maybe write this to memory just in case this fails in the future?
    fk_assert(deviceId.initialize(*services().bus));

    Hardware::cyclePeripherals();

    #if !defined(FK_NATURALIST)
    // We just leave modules on for FkNat.
    Hardware::cycleModules();
    #endif

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

    auto now = clock.now();
    FormattedTime nowFormatted{ now };
    log("Now: %s (%lu)", nowFormatted.toString(), now.unixtime());

    services().watchdog->started();
    services().state->started();
    services().scheduler->begin(lwcron::DateTime{ now.unixtime() });

    transit(services().states->configure);
}

void StartSystem::setupFlash() {
    auto flashFs = services().flashFs;
    auto flashState = services().flashState;

    if (!flashFs->initialize(Hardware::FLASH_PIN_CS, SuperBlockSize)) {
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
