#if defined(FK_NATURALIST)
#include <ArduinoSound.h>
#endif

#include "booting.h"
#include "hardware.h"
#include "leds.h"
#include "watchdog.h"
#include "power_management.h"
#include "user_button.h"
#include "factory_reset_check.h"

namespace fk {

static void dump_configuration() {
    loginfof("Core", "Starting");

    #ifdef FK_DEBUG_UART_FALLBACK
    loginfof("Core", "FK_DEBUG_UART_FALLBACK");
    #endif

    #ifdef FK_DEBUG_MTB_ENABLE
    loginfof("Core", "FK_DEBUG_MTB_ENABLE");
    #endif

    #if defined(FK_NATURALIST)
    loginfof("Core", "FK_NATURALIST");
    #endif

    #if defined(FK_CORE_GENERATION_2)
    loginfof("Core", "FK_CORE_GENERATION_2");
    #endif

    #if defined(FK_CORE_GENERATION_1)
    loginfof("Core", "FK_CORE_GENERATION_1");
    #endif

    #ifdef FK_CORE_REQUIRE_MODULES
    loginfof("Core", "FK_CORE_REQUIRE_MODULES");
    #endif

    #ifdef FK_WIFI_STARTUP_ONLY
    loginfof("Core", "FK_WIFI_STARTUP_ONLY");
    #endif

    #ifdef FK_WIFI_ALWAYS_ON
    loginfof("Core", "FK_WIFI_ALWAYS_ON");
    #endif

    #ifdef FK_DISABLE_RADIO
    loginfof("Core", "FK_DISABLE_RADIO");
    #endif

    #ifdef FK_ENABLE_RADIO
    loginfof("Core", "FK_ENABLE_RADIO");
    #endif

    #ifdef FK_DISABLE_FLASH
    loginfof("Core", "FK_DISABLE_FLASH");
    #endif

    #ifdef FK_ENABLE_FLASH
    loginfof("Core", "FK_ENABLE_FLASH");
    #endif

    #ifdef FK_ENABLE_LOW_POWER_SLEEP
    loginfof("Core", "FK_ENABLE_LOW_POWER_SLEEP");
    #endif

    #ifdef FK_ENABLE_DEEP_SLEEP
    loginfof("Core", "FK_ENABLE_DEEP_SLEEP");
    #endif

    #ifdef FK_DISABLE_LOW_POWER_SLEEP
    loginfof("Core", "FK_DISABLE_LOW_POWER_SLEEP");
    #endif

    #ifdef FK_DISABLE_DEEP_SLEEP
    loginfof("Core", "FK_DISABLE_DEEP_SLEEP");
    #endif

    #ifdef FK_PROFILE_STANDARD
    loginfof("Core", "FK_PROFILE_STANDARD");
    #endif

    #ifdef FK_PROFILE_AMAZON
    loginfof("Core", "FK_PROFILE_AMAZON");
    #endif

    #ifdef FK_DEBUG_INSTRUMENTATION
    loginfof("Core", "FK_DEBUG_INSTRUMENTATION");
    #endif
}

void Booting::task() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    dump_configuration();

    // This only works if I do this before we initialize the WDT, for some
    // reason. Not a huge priority to fix but I'd like to understand why
    // eventually. 44100
    #if defined(FK_NATURALIST)
    log("Initialize I2S");
    fk_assert(AudioInI2S.begin(8000, 32));
    #endif

    services().leds->setup();
    services().watchdog->setup();
    services().bus->begin(400000);
    services().power->setup();
    services().button->enqueued();

    transit<FactoryResetCheck>();
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)