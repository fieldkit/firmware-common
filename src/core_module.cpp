#include "core_module.h"
#include "hardware.h"
#include "device_id.h"

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

class Status : public ActiveObject {
private:
    uint32_t lastTick{ 0 };
    CoreState *state;

public:
    Status(CoreState &state) : state(&state) {
    }

public:
    void idle() override {
        if (millis() - lastTick > 5000) {
            IpAddress4 ip{ state->getStatus().ip };
            debugfpln("Status", "Status (%.2f%% / %.2fmv) (%lu free) (%s)",
                      state->getStatus().batteryPercentage, state->getStatus().batteryVoltage,
                      fk_free_memory(), ip.toString());
            lastTick = millis();
        }
    }

};


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

    fk_assert(setupFileSystem());

    bus.begin();

    DeviceId deviceId{ bus };
    state.setDeviceId(deviceId.toString());

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
    fk::SimpleNTP ntp(clock);
    Status status{ state };

    wifi.begin();

    scheduler.push(ntp);

    while (true) {
        status.tick();
        leds.tick();
        power.tick();
        watchdog.tick();
        liveData.tick();
        wifi.tick();
        // This is ugly.
        if (liveData.isIdle()) {
            scheduler.tick();
        }
        discovery.tick();
        attachedDevices.tick();
    }
}

}

