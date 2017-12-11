#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <Arduino.h>

#include <RH_RF95.h>

#include <sd_raw.h>
#include <fkfs.h>
#include <fkfs_log.h>

#include <fk-core.h>

#include "config.h"
#include "seed.h"

extern "C" {

const uint8_t WIFI_PIN_CS = 8;

const uint8_t RFM95_PIN_CS = 8;
const uint8_t RFM95_PIN_RST = 4;
const uint8_t RFM95_PIN_INT = 3;

const uint8_t SD_PIN_CS = 10;
const uint8_t FKFS_FILE_LOG = 0;
const uint8_t FKFS_FILE_PRIORITY_LOWEST = 255;
const uint8_t FKFS_FILE_PRIORITY_HIGHEST = 0;

static fkfs_t fs = { 0 };
static fkfs_log_t fkfs_log = { 0 };

void debug_write_log(const char *str, void *arg) {
    fkfs_log_append(&fkfs_log, str);
}

size_t fkfs_log_message(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debug_configure_hook(false);
    vdebugfpln("fkfs", f, args);
    debug_configure_hook(true);
    va_end(args);
    return 0;
}

bool setupLogging() {
    fkfs_configure_logging(fkfs_log_message);

    if (!fkfs_create(&fs)) {
        return false;
    }

    randomSeed(RANDOM_SEED);

    if (!sd_raw_initialize(&fs.sd, SD_PIN_CS)) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_LOG, FKFS_FILE_PRIORITY_LOWEST, true, "FK.LOG")) {
        return false;
    }

    auto wipe = true;
    if (!fkfs_initialize(&fs, wipe)) {
        return false;
    }

    fkfs_log_statistics(&fs);

    if (!fkfs_log_initialize(&fkfs_log, &fs, FKFS_FILE_LOG)) {
        return false;
    }

    debug_add_hook(debug_write_log, &fkfs_log);

    return true;
}

void synchronizeClock(fk::Clock &clock, fk::Wifi &wifi, fk::Watchdog &watchdog) {
    fk::SimpleNTP ntp(clock);

    while (!ntp.task().isDone()) {
        watchdog.tick();
        wifi.tick();
    }
}

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    debugfpln("Core", "Starting (%d free)", fk_free_memory());

    pinMode(SD_PIN_CS, OUTPUT);
    pinMode(WIFI_PIN_CS, OUTPUT);
    digitalWrite(SD_PIN_CS, HIGH);
    digitalWrite(WIFI_PIN_CS, HIGH);

    delay(10);

    if (!setupLogging()) {
        debugfpln("Core", "No sd");
        while (true) {
            delay(10);
        }
    }

    fk::i2c_begin();

    fk::Watchdog watchdog;
    fk::CoreState state;
    fk::Clock clock;

    clock.begin();

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
    FK_DUMP_SIZE(fs);
    FK_DUMP_SIZE(fkfs_log);
#endif

    uint8_t addresses[]{ 7, 8, 9, 0 };
    {
        fk::Pool pool("SCAN", 128);
        fk::AttachedDevices ad(addresses, state, pool);
        ad.scan();

        while (true) {
            watchdog.tick();
            ad.tick();

            if (ad.isIdle()) {
                break;
            }
        }
    }

    fk::Pool modulesPool("ModulesPool", 128);
    fk::Pool appPool("AppPool", 128);

    fk::NetworkSettings networkSettings {
        .ssid = FK_CONFIG_WIFI_SSID,
        .password = FK_CONFIG_WIFI_PASSWORD,
        .port = FK_CONFIG_WIFI_PORT,
    };

    fk::HttpTransmissionConfig transmissionConfig = {
        .url = "http://code.conservify.org/ingestion"
    };
    fk::HttpPost transmission(transmissionConfig);
    fk::GatherReadings gatherReadings(state, modulesPool);
    fk::SendTransmission sendTransmission(state, transmission, modulesPool);
    fk::SendStatus sendStatus(state, transmission, modulesPool);
    fk::DetermineLocation determineLocation(state, modulesPool);
    fk::ScheduledTask tasks[] = {
        fk::ScheduledTask{ { -1, 30 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, gatherReadings },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendTransmission },
        fk::ScheduledTask{ {  0, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, sendStatus },
        fk::ScheduledTask{ { 10, -1 }, { -1, -1 }, { -1, -1 }, { -1, -1 }, determineLocation },
    };
    fk::Scheduler scheduler(state, clock, tasks);

    fk::LiveData liveData(state, modulesPool);
    fk::FkfsReplies fileReplies(fs);
    fk::AppServicer appServicer(liveData, state, scheduler, fileReplies, appPool);
    fk::Wifi wifi(networkSettings, appServicer);

    // TODO: Fix that this is blocking when connecting.
    wifi.begin();

    synchronizeClock(clock, wifi, watchdog);

    {
        while (true) {
            watchdog.tick();
            liveData.tick();
            wifi.tick();
            scheduler.tick();
        }
    }
}

void loop() {
}

}
