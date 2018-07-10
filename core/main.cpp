/**
 * @file
 */
#include <Arduino.h>

#include <fk-core.h>

#include "config.h"
#include "platform.h"
#include "seed.h"
#include "device_id.h"
#include "two_wire.h"
#include "fkfs_data.h"
#include "restart_wizard.h"

extern "C" {

// #define DEBUG_MTB_ENABLE
// #define DEBUG_DUMP_OBJECT_SIZES
// #define DEBUG_UART_FALLBACK

#ifdef DEBUG_MTB_ENABLE
#define DEBUG_MTB_SIZE 256
__attribute__((__aligned__(DEBUG_MTB_SIZE * sizeof(uint32_t)))) uint32_t mtb[DEBUG_MTB_SIZE];

void HardFault_Handler(void) {
    // Turn off the micro trace buffer so we don't fill it up in the infinite loop below.
    REG_MTB_MASTER = 0x00000000 + 6;
    while (true) {
    }
}
#endif

void dumpObjectSizes();

void setup() {
    #ifdef DEBUG_MTB_ENABLE
    REG_MTB_POSITION = ((uint32_t) (mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    REG_MTB_FLOW = ((uint32_t) mtb + DEBUG_MTB_SIZE * sizeof(uint32_t)) & 0xFFFFFFF8;
    REG_MTB_MASTER = 0x80000000 + 6;
    #endif

    Serial.begin(115200);

    while (!Serial) {
        delay(100);

        #ifndef DEBUG_UART_REQUIRE_CONSOLE
        if (fk::fk_uptime() > 2000) {
            break;
        }
        #endif
    }

    #ifdef DEBUG_UART_FALLBACK
    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }
    #endif

    randomSeed(RANDOM_SEED);
    firmware_version_set(FIRMWARE_GIT_HASH);
    firmware_build_set(FIRMWARE_BUILD);

    loginfof("Core", "Starting");
    #ifdef DEBUG_UART_FALLBACK
    loginfof("Core", "Configured with UART fallback.");
    #endif
    #ifdef DEBUG_MTB_ENABLE
    loginfof("Core", "Configured with MTB.");
    #endif

    #if defined(FK_NATURALIST)
    loginfof("Core", "FK_NATURALIST");
    #elif defined(FK_CORE_GENERATION_2)
    loginfof("Core", "FK_CORE_GENERATION_2");
    #elif defined(FK_CORE_GENERATION_1)
    loginfof("Core", "FK_CORE_GENERATION_1");
    #endif

    fk::NetworkInfo networks[] = {
        {
            FK_CONFIG_WIFI_1_SSID,
            FK_CONFIG_WIFI_1_PASSWORD,
        },
        {
            FK_CONFIG_WIFI_2_SSID,
            FK_CONFIG_WIFI_2_PASSWORD,
        }
    };

    fk::CoreModule coreModule;
    coreModule.begin();
    fk::restartWizard.startup();
    auto startupConfig = fk::StartupConfigurer{ coreModule.getState() };
    if (false) {
        startupConfig.overrideEmptyNetworkConfigurations(fk::NetworkSettings{ false, networks });
    }
    else {
        startupConfig.forceConfiguration(fk::NetworkSettings{ false, networks });
    }
    coreModule.run();
}

void loop() {
}

}
