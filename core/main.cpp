#include <Arduino.h>

#include <fk-core.h>

#include "config.h"
#include "seed.h"
#include "device_id.h"
#include "i2c.h"
#include "fkfs_data.h"

extern "C" {

// #define DEBUG_MTB_ENABLE

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

    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        // USBDevice.detach(); Leave this so we can program w/o the programmer.
        Serial5.begin(115200);
        debug_uart_set(Serial5);
    }

    debugfpln("Core", "Starting");

    firmware_version_set(FIRMWARE_GIT_HASH);

    fk::TwoWireBus bus{ Wire };
    bus.begin();

    randomSeed(RANDOM_SEED);

    {
        fk::SerialNumber serialNumber;
        fk::DeviceId deviceId{ bus };
        debugfpln("Core", "Serial(%s)", serialNumber.toString());
        debugfpln("Core", "DeviceId(%s)", deviceId.toString());
        debugfpln("Core", "Hash(%s)", FIRMWARE_GIT_HASH);
    }

#define FK_DUMP_OBJECT_SIZES
#ifdef FK_DUMP_OBJECT_SIZES
#define FK_DUMP_SIZE(K)  debugfpln("Core", "%s: %d", #K, sizeof(K))
    FK_DUMP_SIZE(fk::Watchdog);
    FK_DUMP_SIZE(fk::CoreState);
    FK_DUMP_SIZE(fk::Clock);
    FK_DUMP_SIZE(fk::Pool);
    FK_DUMP_SIZE(fk::HttpPost);
    FK_DUMP_SIZE(fk::GatherReadings);
    FK_DUMP_SIZE(fk::SendTransmission);
    FK_DUMP_SIZE(fk::SendStatus);
    FK_DUMP_SIZE(fk::DetermineLocation);
    FK_DUMP_SIZE(fk::Scheduler);
    FK_DUMP_SIZE(fk::LiveData);
    FK_DUMP_SIZE(fk::AppServicer);
    FK_DUMP_SIZE(fk::Wifi);
    FK_DUMP_SIZE(fk::SimpleNTP);
    FK_DUMP_SIZE(WiFiUDP);
    FK_DUMP_SIZE(fkfs_t);
    FK_DUMP_SIZE(fkfs_header_t);
    FK_DUMP_SIZE(fkfs_log_t);
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
    coreModule.getState().configure(fk::NetworkSettings{ false, networks });

    coreModule.run();
}

void loop() {
}

}
