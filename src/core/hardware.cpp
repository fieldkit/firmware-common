#include <SPI.h>
#include <Arduino.h>

#include "hardware.h"
#include "debug.h"
#include "configuration.h"
#include "two_wire.h"

namespace fk {

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

uint32_t Hardware::modules_on_at_ = 0;
uint32_t Hardware::peripherals_on_at_ = 0;

#if defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

Uart &Hardware::gpsUart = Serial2;

#else // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

Uart &Hardware::gpsUart = Serial1;

#endif // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

void Hardware::enableModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Enable modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    modules_on_at_ = fk_uptime();
    #endif

    enableModuleI2c();
    delay(500);
}

void Hardware::disableModules() {
    disableModuleI2c();

    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Disabling modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, LOW);
    modules_on_at_ = 0;
    #endif
}

void Hardware::cycleModules() {
    #if defined(FK_CORE_GENERATION_2)
    disableModules();
    delay(500);
    enableModules();
    #else
    Logger::info("Modules always on.");
    #endif
}

void Hardware::enablePeripherals() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::trace("Enabling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, HIGH);
    peripherals_on_at_ = fk_uptime();
    #endif

    enableSpi();
    delay(500);
}

void Hardware::disablePeripherals() {
    disableSpi();

    #if defined(FK_CORE_GENERATION_2)
    Logger::trace("Disabling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, LOW);
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    peripherals_on_at_ = 0;
    #endif
}

bool Hardware::peripheralsEnabled() {
    #if defined(FK_CORE_GENERATION_2)
    return digitalRead(PERIPHERALS_ENABLE_PIN) == HIGH;
    #else
    return true;
    #endif
}

void Hardware::cyclePeripherals() {
    #if defined(FK_CORE_GENERATION_2)
    disablePeripherals();
    delay(500);
    enablePeripherals();
    #else
    Logger::info("Peripherals always on.");
    #endif
}

void Hardware::enableGps() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Enabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, HIGH);
    #endif
}

void Hardware::disableGps() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Disabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    #endif
}

bool Hardware::modulesReady() {
    return modules_on_at_ > 0 && fk_uptime() - modules_on_at_ > configuration.modules_ready_time;
}

void Hardware::enableSpi() {
    uint8_t pins[] = { FLASH_PIN_CS, SD_PIN_CS, WIFI_PIN_CS, RFM95_PIN_CS };
    for (auto pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    SPI.begin();
}

void Hardware::disableSpi() {
    uint8_t pins[] = {
        FLASH_PIN_CS, SD_PIN_CS, WIFI_PIN_CS, RFM95_PIN_CS,
        SPI_PIN_MISO, SPI_PIN_MOSI, SPI_PIN_SCK,
    };
    for (auto pin : pins) {
        pinMode(pin, INPUT);
    }
}

void Hardware::enableModuleI2c() {
    TwoWireBus bus2{ Wire4and3 };
    bus2.begin(400000);
}

void Hardware::disableModuleI2c() {
    pinMode(I2C_PIN_SDA2, INPUT);
    pinMode(I2C_PIN_SCL2, INPUT);
}

}

extern "C" {

void fk_assertion_hook(void) {
    fk::Hardware::disableModules();
    fk::Hardware::disablePeripherals();
}

}
