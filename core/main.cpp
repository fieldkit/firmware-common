/**
 * @file
 */
#include <Arduino.h>

#include <fk-core.h>

#include "config.h"
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

    while (!Serial && millis() < 2000) {
        delay(100);
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

    #ifdef DEBUG_DUMP_OBJECT_SIZES
    dumpObjectSizes();
    #endif

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

void dumpObjectSizes() {
#define FK_DUMP_SIZE(K) loginfof("Core", "%s: %d", #K, sizeof(K))
    FK_DUMP_SIZE(fk::Watchdog);
    FK_DUMP_SIZE(fk::CoreState);
    FK_DUMP_SIZE(fk::Clock);
    FK_DUMP_SIZE(fk::Pool);
    FK_DUMP_SIZE(fk::GatherReadings);
    FK_DUMP_SIZE(fk::Scheduler);
    FK_DUMP_SIZE(fk::LiveData);
    FK_DUMP_SIZE(fk::AppServicer);
    FK_DUMP_SIZE(fk::Wifi);
    FK_DUMP_SIZE(fk::Listen);
    FK_DUMP_SIZE(fk::TwoWireMessageBuffer);
    FK_DUMP_SIZE(fk::WifiMessageBuffer);
    FK_DUMP_SIZE(fk::TransmitFileTask);
    FK_DUMP_SIZE(fk::ScanNetworks);
    FK_DUMP_SIZE(fk::ConnectToWifiAp);
    FK_DUMP_SIZE(fk::Delay);
    FK_DUMP_SIZE(fk::SimpleNTP);
    FK_DUMP_SIZE(fk::Discovery);
    FK_DUMP_SIZE(fk::DataRecordMessage);
    FK_DUMP_SIZE(fk::AppReplyMessage);
    FK_DUMP_SIZE(fk::AppQueryMessage);
    FK_DUMP_SIZE(fk::ModuleReplyMessage);
    FK_DUMP_SIZE(fk::ModuleQueryMessage);
    FK_DUMP_SIZE(fk::AppServicer);
    FK_DUMP_SIZE(fk::AppModuleQueryTask);
    FK_DUMP_SIZE(fk_module_WireMessageQuery);
    FK_DUMP_SIZE(fk_module_WireMessageReply);
    FK_DUMP_SIZE(fk_app_WireMessageQuery);
    FK_DUMP_SIZE(fk_app_WireMessageReply);
    FK_DUMP_SIZE(WiFi);
    FK_DUMP_SIZE(WiFiUDP);
    FK_DUMP_SIZE(WiFiClient);
    FK_DUMP_SIZE(WiFiServer);
}

void loop() {
}

}
