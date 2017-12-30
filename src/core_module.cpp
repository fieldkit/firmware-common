#include "core_module.h"
#include "hardware.h"

namespace fk {

extern "C" {

static size_t debug_write_log(const char *str, void *arg) {
    if (!fkfs_log_append((fkfs_log_t *)arg, str)) {
        debug_uart_get()->println("Unable to append log");
    }
    return 0;
}

static size_t fkfs_log_message(const char *f, ...) {
    va_list args;
    va_start(args, f);
    debug_configure_hook(false);
    vdebugfpln("fkfs", f, args);
    debug_configure_hook(true);
    va_end(args);
    return 0;
}

}

CoreModule::CoreModule() {
}

void CoreModule::begin() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);
    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    leds.setup();
    watchdog.setup();
    power.setup();

    delay(10);

    if (!setupFileSystem()) {
        debugfpln("Core", "No sd (%d)", Hardware::SD_PIN_CS);
        while (true) {
            delay(10);
        }
    }

    i2c_begin();

    clock.begin();
}

bool CoreModule::setupFileSystem() {
    fkfs_configure_logging(fkfs_log_message);

    if (!fkfs_create(&fs)) {
        return false;
    }

    if (!sd_raw_initialize(&fs.sd, Hardware::SD_PIN_CS)) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_LOG, FKFS_FILE_PRIORITY_LOWEST, true, "FK.LOG")) {
        return false;
    }

    if (!fkfs_initialize_file(&fs, FKFS_FILE_DATA, FKFS_FILE_PRIORITY_HIGHEST, true, "DATA.BIN")) {
        return false;
    }

    auto wipe = false;
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

void CoreModule::run() {
    uint8_t addresses[]{ 7, 8, 9, 0 };
    {
        fk::Pool pool("SCAN", 256);
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

    if (state.numberOfModules() == 0) {
        leds.noAttachedModules();
    }

    // TODO: Fix that this is blocking when connecting.
    wifi.begin();

    fk::SimpleNTP ntp(clock);

    scheduler.push(ntp);

    {
        while (true) {
            constexpr int32_t NumberOfTimes = 8;
            uint32_t times[NumberOfTimes] = { 0 };
            auto i = 0;

            times[i++] = millis();
            leds.tick();
            times[i++] = millis();
            power.tick();
            times[i++] = millis();
            watchdog.tick();
            times[i++] = millis();
            liveData.tick();
            times[i++] = millis();
            wifi.tick();
            times[i++] = millis();
            scheduler.tick();
            times[i++] = millis();
            discovery.task();
            times[i++] = millis();

            auto diff = times[NumberOfTimes - 1] - times[0];
            if (diff > 500) {
                debugfpln("Core", "Long Tick: %lu", diff);
                for (auto i = 1; i < NumberOfTimes; ++i) {
                    diff = times[i] - times[0];
                    if (diff > 0) {
                        debugfpln("Core", "Time[%d]: %lu", i, diff);
                    }
                }
            }
        }
    }
}

}

