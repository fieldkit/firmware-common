#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <Arduino.h>

#include <RH_RF95.h>

#include <sd_raw.h>
#include <fkfs.h>
#include <fkfs_log.h>

#include "app_servicer.h"
#include "attached_devices.h"
#include "debug.h"
#include "i2c.h"
#include "module_controller.h"
#include "pool.h"
#include "wifi.h"

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

RH_RF95 rf95(RFM95_PIN_CS, RFM95_PIN_INT); 

void debug_write_log(const char *str, void *arg) {
    fkfs_log_append(&fkfs_log, str);
}

size_t fkfs_log_message(const char *f, ...) {
    va_list args;
    va_start(args, f);
    vdebugfpln("fkfs", f, args);
    va_end(args);
    return 0;
}

bool setupRadio() {
    if (!rf95.init()) {
        debugfpln("Core", "LoRa missing (%d)", sizeof(rf95));
    }

    return false;
}

bool setupLogging() {
    fkfs_configure_logging(fkfs_log_message);

    if (!fkfs_create(&fs)) {
        return false;
    }

    pinMode(SD_PIN_CS, OUTPUT);
    digitalWrite(SD_PIN_CS, HIGH);

    if (!sd_raw_initialize(&fs.sd, SD_PIN_CS)) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_LOG, FKFS_FILE_PRIORITY_LOWEST, false, "DEBUG.LOG")) {
        return false;
    }

    if (!fkfs_log_initialize(&fkfs_log, &fs, FKFS_FILE_LOG)) {
        return false;
    }

    if (false) {
        if (!fkfs_initialize(&fs, true)) {
            return false;
        }
        fkfs_log_statistics(&fs);
    }

    if (!fkfs_initialize(&fs, false)) {
        return false;
    }
    fkfs_log_statistics(&fs);

    debug_add_hook(debug_write_log, &fkfs_log);

    return true;
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

    if (!setupRadio()) {

    }

    fk::i2c_begin();

    {
        uint8_t addresses[]{ 7, 8, 9, 0 };
        fk::Pool pool("ROOT", 128);
        fk::AttachedDevices ad(addresses, 0, pool);
        ad.scan();

        while (true) {
            ad.tick();

            if (ad.idle()) {
                break;
            }
        }
    }

    debugfpln("Core", "Idle");

    {
        fk::Pool pool("ROOT", 128);
        fk::ModuleController modules(8, pool);
        fk::NetworkSettings networkSettings {
            .ssid = "Cottonwood",
            .password = "asdfasdf",
            .port = 54321,
        };
        fk::Wifi wifi(networkSettings, modules);

        // TODO: Fix that this is blocking when connecting.
        wifi.begin();

        modules.beginReading();

        while (true) {
            modules.tick();
            wifi.tick();
        }
    }
}

void loop() {
}

}
