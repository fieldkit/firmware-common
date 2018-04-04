#include "core_module.h"
#include "hardware.h"
#include "device_id.h"
#include "status.h"

namespace fk {

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
    bus.begin();
    power.setup();

    fk_assert(deviceId.initialize(bus));

    SerialNumber serialNumber;
    debugfpln("Core", "Serial(%s)", serialNumber.toString());
    debugfpln("Core", "DeviceId(%s)", deviceId.toString());
    debugfpln("Core", "Hash(%s)", firmware_version_get());

    delay(10);

    #ifndef FK_DISABLE_FLASH
    fk_assert(serialFlash.begin(Hardware::FLASH_PIN_CS));
    fk_assert(storage.setup());

    delay(100);
    #endif

    fk_assert(fileSystem.setup());

    watchdog.started();

    bus.begin();

    state.setDeviceId(deviceId.toString());

    clock.begin();

    FormattedTime nowFormatted{ clock.now() };
    debugfpln("Core", "Now: %s", nowFormatted.toString());

    state.started();
}

void CoreModule::run() {
    SimpleNTP ntp(clock, wifi);
    Status status{ state, bus };

    wifi.begin();

    supervisor.push(ntp);

    // TODO: Can this just take references?
    auto tasks = to_parallel_task_collection(
        &status,
        &leds,
        &power,
        &watchdog,
        &attachedDevices,
        &scheduler,
        &wifi,
        &discovery,
        &liveData,
        &moduleCommunications,
        &supervisor
    );

    while (true) {
        tasks.task();
    }
}

}

