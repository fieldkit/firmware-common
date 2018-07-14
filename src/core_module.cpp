#include "core_module.h"
#include "hardware.h"
#include "device_id.h"
#include "status.h"

namespace fk {

constexpr const char LogName[] = "Core";

using Logger = SimpleLog<LogName>;

void CoreModule::begin() {
    MainServicesState::services(mainServices);
    WifiServicesState::services(wifiServices);

    fsm_list::start();

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
    button.enqueued();

    fk_assert(deviceId.initialize(bus));

    SerialNumber serialNumber;
    Logger::info("Serial(%s)", serialNumber.toString());
    Logger::info("DeviceId(%s)", deviceId.toString());
    Logger::info("Hash(%s)", firmware_version_get());
    Logger::info("Build(%s)", firmware_build_get());
    Logger::info("API(%s)", WifiApiUrlIngestionStream);

    delay(10);

    #ifdef FK_CORE_GENERATION_2
    Logger::info("Cycling peripherals.");
    pinMode(Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, LOW);
    delay(500);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, HIGH);
    delay(500);
    #else
    Logger::info("Peripherals always on.");
    #endif

    #ifdef FK_ENABLE_FLASH
    fk_assert(flashStorage.initialize(Hardware::FLASH_PIN_CS));
    #else
    Logger::info("Flash memory disabled");
    #endif

    #ifdef FK_ENABLE_RADIO
    if (!radioService.setup(deviceId)) {
        Logger::info("Radio service unavailable");
    }
    else {
        Logger::info("Radio service ready");
    }
    #else
    Logger::info("Radio service disabled");
    #endif

    fk_assert(fileSystem.setup());

    watchdog.started();

    bus.begin();

    state.setDeviceId(deviceId.toString());

    clock.begin();

    scheduler.setup();

    FormattedTime nowFormatted{ clock.now() };
    Logger::info("Now: %s", nowFormatted.toString());

    state.started();
}

void CoreModule::run() {
    while (true) {
        CoreDevice::current().task();
    }
}

}

