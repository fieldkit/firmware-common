/**
 * @file
 */
#include <Arduino.h>

#include <fk-core.h>

#include "platform.h"
#include "restart_wizard.h"
#include "initialized.h"

#include "seed.h"
#include "config.h"

extern "C" {

namespace fk {

class ConfigureDevice : public fk::MainServicesState {
public:
    const char *name() const override {
        return "ConfigureDevice";
    }

public:
    void entry() override {
        #if defined(FK_CONFIG_WIFI_1_SSID) && defined(FK_CONFIG_WIFI_2_SSID)
        fk::NetworkInfo networks[2] = {
            {
                FK_CONFIG_WIFI_1_SSID,
                FK_CONFIG_WIFI_1_PASSWORD,
            },
            {
                FK_CONFIG_WIFI_2_SSID,
                FK_CONFIG_WIFI_2_PASSWORD,
            }
        };

        services().state->configure(fk::NetworkSettings{ false, networks });

        log("Configured compile time networks.");
        #endif

        transit<fk::Initialized>();
    }
};

}

static void setup_serial();
static void setup_env();
static void dump_configuration();

void setup() {
    #ifdef FK_DEBUG_MTB_ENABLE
    REG_MTB_POSITION = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    REG_MTB_FLOW = ((uint32_t) mtb + DEBUG_MTB_SIZE * sizeof(uint32_t)) & 0xFFFFFFF8;
    REG_MTB_MASTER = 0x80000000 + 6;
    #endif

    setup_serial();
    setup_env();
    dump_configuration();

    fk::restartWizard.startup();
}

void loop() {
    fk::CoreModule coreModule;
    coreModule.run(fk::CoreFsm::deferred<fk::ConfigureDevice>());
}

static void setup_serial() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);

        #ifndef FK_DEBUG_UART_REQUIRE_CONSOLE
        if (fk::fk_uptime() > 2000) {
            break;
        }
        #endif
    }

    #ifdef FK_DEBUG_UART_FALLBACK
    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }
    #endif
}

static void setup_env() {
    randomSeed(RANDOM_SEED);
    firmware_version_set(FIRMWARE_GIT_HASH);
    firmware_build_set(FIRMWARE_BUILD);
    firmware_compiled_set(DateTime(__DATE__, __TIME__).unixtime());
}

static void dump_configuration() {
    loginfof("Core", "Starting");

    #ifdef FK_DEBUG_UART_FALLBACK
    loginfof("Core", "FK_DEBUG_UART_FALLBACK");
    #endif

    #ifdef FK_DEBUG_MTB_ENABLE
    loginfof("Core", "FK_DEBUG_MTB_ENABLE");
    #else
    loginfof("Core", "FK_DEBUG_MTB_DISABLE");
    #endif

    #if defined(FK_NATURALIST)
    loginfof("Core", "FK_NATURALIST");
    #elif defined(FK_CORE_GENERATION_2)
    loginfof("Core", "FK_CORE_GENERATION_2");
    #elif defined(FK_CORE_GENERATION_1)
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

#ifdef FK_DEBUG_MTB_ENABLE

#define DEBUG_MTB_SIZE 256
__attribute__((__aligned__(DEBUG_MTB_SIZE * sizeof(uint32_t)))) uint32_t mtb[DEBUG_MTB_SIZE];

void HardFault_Handler(void) {
    // Turn off the micro trace buffer so we don't fill it up in the infinite loop below.
    REG_MTB_MASTER = 0x00000000 + 6;
    while (true) {
    }
}

#endif

}
