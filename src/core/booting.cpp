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

    #ifdef FK_CORE_REQUIRE_MODULES
    loginfof("Core", "FK_CORE_REQUIRE_MODULES");
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

    #if defined(FK_ENABLE_FUEL_GAUGE)
    loginfof("Core", "FK_ENABLE_FUEL_GAUGE");
    #endif

    #if defined(FK_DISABLE_FUEL_GAUGE)
    loginfof("Core", "FK_DISABLE_FUEL_GAUGE");
    #endif

    #ifdef FK_ENABLE_LOW_POWER_SLEEP
    loginfof("Core", "FK_ENABLE_LOW_POWER_SLEEP");
    #endif

    #ifdef FK_ENABLE_DEEP_SLEEP
    loginfof("Core", "FK_ENABLE_DEEP_SLEEP");
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

    #ifdef FK_GPS_FIXED_STATION
    loginfof("Core", "FK_GPS_FIXED_STATION");
    #endif
}

void Booting::task() {
    Hardware::disableModules();
    Hardware::disablePeripherals();

    dump_configuration();

    #if defined(FK_NATURALIST)
    // This is required because the FkNat sensors are on the main I2C bus and
    // powered from MODULE_3V3. Mixing them was a bad idea.
    Hardware::enableModules();
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
