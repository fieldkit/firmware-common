#include <lwcron/lwcron.h>

#include "startup.h"
#include "core_state.h"
#include "hardware.h"
#include "leds.h"
#include "device_id.h"
#include "rtc.h"
#include "tuning.h"
#include "watchdog.h"
#include "debug.h"
#include "file_system.h"
#include "user_button.h"
#include "power_management.h"
#include "utils.h"
#include "radio_service.h"

namespace fk {

Booting::Deferred Booting::configure_;

void Booting::task() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    // This only works if I do this before we initialize the WDT, for some
    // reason. Not a huge priority to fix but I'd like to understand why
    // eventually.
    // 44100
    // NOTE: FkNaturalist Specific
    // fk_assert(AudioInI2S.begin(8000, 32));

    services().leds->setup();
    services().watchdog->setup();
    services().bus->begin(400000);
    services().power->setup();
    services().button->enqueued();

    clock.begin();

    // TODO: Maybe write this to memory just in case this fails in the future?
    fk_assert(deviceId.initialize(*services().bus));
    services().state->setDeviceId(deviceId.toString());

    #ifdef FK_CORE_GENERATION_2
    log("Cycling peripherals.");
    pinMode(Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, LOW);
    delay(500);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, HIGH);
    delay(500);
    #else
    log("Peripherals always on.");
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
    log("API(%s)", WifiApiUrlIngestionStream);

    auto now = clock.now();
    FormattedTime nowFormatted{ now };
    log("Now: %s (%lu)", nowFormatted.toString(), now.unixtime());

    services().watchdog->started();
    services().state->started();
    services().scheduler->begin(lwcron::DateTime{ now.unixtime() });

    transit(configure_);
}

void Booting::setupFlash() {
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

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
