#include "debug.h"
#include "hardware.h"

#include <Arduino.h>

const char *global_firmware_build = "<unknown>";

const char *firmware_build_get() {
    return global_firmware_build;
}

void firmware_build_set(const char *hash) {
    global_firmware_build = hash;
}

const char *global_firmware_version = "<unknown>";

const char *firmware_version_get() {
    return global_firmware_version;
}

void firmware_version_set(const char *hash) {
    global_firmware_version = hash;
}

uint32_t global_compiled = 0;

uint32_t firmware_compiled_get() {
    return global_compiled;
}

void firmware_compiled_set(uint32_t compiled) {
    global_compiled = compiled;
}

extern "C" char *sbrk(int32_t i);

uint32_t fk_free_memory() {
    char stack_dummy = 0;
    return &stack_dummy - sbrk(0);
}

void __fk_assert(const char *msg, const char *file, int lineno) {
    loginfof("Assert", "ASSERTION: %s:%d '%s'", file, lineno, msg);

    #ifdef ARDUINO
    log_uart_get()->flush();
    #endif

    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    #ifdef FK_CORE_GENERATION_2
    loginfof("Assert", "Disabling peripherals.");
    pinMode(fk::Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(fk::Hardware::PIN_PERIPH_ENABLE, LOW);
    #endif

    while (1) {
        // This will happen until the WDT kicks is back to Reset, hopefully. I'm
        // flashing these just to sort of give the user an idea that something's
        // going wrong if they happen to be around.
        digitalWrite(A3, LOW);
        digitalWrite(A4, LOW);
        digitalWrite(A5, LOW);
        delay(100);
        digitalWrite(A3, HIGH);
        digitalWrite(A4, HIGH);
        digitalWrite(A5, HIGH);
        delay(100);
    }
}
