#include <Arduino.h>

#include "hardware.h"
#include "debug.h"
#include "configuration.h"

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

void Hardware::disableModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Disabling modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, LOW);
    modules_on_at_ = 0;
    #endif
}

void Hardware::enableModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Enable modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    modules_on_at_ = fk_uptime();
    #endif
}

void Hardware::cycleModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Cycling modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, LOW);
    delay(500);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    modules_on_at_ = fk_uptime();
    delay(500);
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
}

void Hardware::disablePeripherals() {
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
    Logger::trace("Cycling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, LOW);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    delay(500);
    digitalWrite(PERIPHERALS_ENABLE_PIN, HIGH);
    digitalWrite(GPS_ENABLE_PIN, HIGH);
    peripherals_on_at_ = fk_uptime();
    delay(500);
    #else
    Logger::info("Peripherals always on.");
    #endif
}

void Hardware::disableGps() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Disabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, LOW);
    #endif
}

void Hardware::enableGps() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Enabling GPS.");
    pinMode(GPS_ENABLE_PIN, OUTPUT);
    digitalWrite(GPS_ENABLE_PIN, HIGH);
    #endif
}

bool Hardware::modulesReady() {
    return modules_on_at_ > 0 && fk_uptime() - modules_on_at_ > configuration.modules_ready_time;
}

}

extern "C" {

void fk_assertion_hook(void) {
    fk::Hardware::disableModules();
    fk::Hardware::disablePeripherals();
}

}
