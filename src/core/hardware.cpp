#include <Arduino.h>

#include "hardware.h"
#include "debug.h"

namespace fk {

constexpr const char Log[] = "Hardware";

using Logger = SimpleLog<Log>;

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
    #endif
}

void Hardware::enableModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Enable modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    #endif
}

void Hardware::cycleModules() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Cycling modules.");
    pinMode(MODULES_ENABLE_PIN, OUTPUT);
    digitalWrite(MODULES_ENABLE_PIN, LOW);
    delay(500);
    digitalWrite(MODULES_ENABLE_PIN, HIGH);
    delay(500);
    #else
    Logger::info("Modules always on.");
    #endif
}

void Hardware::disablePeripherals() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Disabling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, LOW);
    #endif
}

void Hardware::cyclePeripherals() {
    #if defined(FK_CORE_GENERATION_2)
    Logger::info("Cycling peripherals.");
    pinMode(PERIPHERALS_ENABLE_PIN, OUTPUT);
    digitalWrite(PERIPHERALS_ENABLE_PIN, LOW);
    delay(500);
    digitalWrite(PERIPHERALS_ENABLE_PIN, HIGH);
    delay(500);
    #else
    Logger::info("Peripherals always on.");
    #endif
}

}

extern "C" {

void fk_assertion_hook(void) {
    fk::Hardware::disableModules();
    fk::Hardware::disablePeripherals();
}

}
