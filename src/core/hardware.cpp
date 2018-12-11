#include <SPI.h>
#include <Arduino.h>

#include "hardware.h"
#include "debug.h"
#include "configuration.h"
#include "serial_port.h"
#include "two_wire.h"

namespace fk {

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

uint32_t Hardware::modules_on_at_ = 0;
uint32_t Hardware::peripherals_on_at_ = 0;

Uart &Hardware::gpsUart = Serial2;

void Hardware::enableModules() {
    Logger::info("Enable modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    modules_on_at_ = fk_uptime();

    enableModuleI2c();
    delay(500);
}

void Hardware::disableModules() {
    disableModuleI2c();

    Logger::trace("Disabling modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, LOW);
    modules_on_at_ = 0;
}

void Hardware::cycleModules() {
    disableModules();
    delay(500);
    enableModules();
}

void Hardware::enablePeripherals() {
    Logger::trace("Enabling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, HIGH);
    peripherals_on_at_ = fk_uptime();

    enableSpi();
    delay(500);
}

void Hardware::disablePeripherals() {
    disableSpi();

    Logger::trace("Disabling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, LOW);
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    peripherals_on_at_ = 0;
}

bool Hardware::peripheralsEnabled() {
    return digitalRead(PERIPHERALS_ENABLE_PIN) == HIGH;
}

void Hardware::cyclePeripherals() {
    disablePeripherals();
    delay(500);
    enablePeripherals();
}

void Hardware::enableGps() {
    Logger::info("Enabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, HIGH);
}

void Hardware::disableGps() {
    Logger::info("Disabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
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
    TwoWireBus bus2{ Wire4and3 };
    bus2.end();
}

}

extern "C" {

void fk_assertion_hook(void) {
    fk::Hardware::disableModules();
    fk::Hardware::disablePeripherals();
}

}
