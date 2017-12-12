#include <Arduino.h>

#include <fk-core.h>

#include "config.h"
#include "seed.h"

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    randomSeed(RANDOM_SEED);

    debugfpln("Core", "Starting (%lu free)", fk_free_memory());

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

    fk::CoreModule coreModule;
    coreModule.begin();
    coreModule.configure(fk::NetworkSettings {
            .ssid = FK_CONFIG_WIFI_SSID,
            .password = FK_CONFIG_WIFI_PASSWORD,
            .port = FK_CONFIG_WIFI_PORT,
        }
    );

    coreModule.run();
}

void loop() {
}

}
