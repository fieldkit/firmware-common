#include "debug.h"

#if defined(ARDUINO)
#include "leds.h"
#include <Arduino.h>
#endif

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

extern "C" {

void fk_assertion_hook_dummy()  {
}

void fk_assertion_hook(void) __attribute__ ((weak, alias("fk_assertion_hook_dummy")));

}

void __fk_assert(const char *msg, const char *file, int lineno) {
    fk_assertion_hook();

    #if defined(ARDUINO)

    loginfof("Assert", "ASSERTION: %s:%d '%s'", file, lineno, msg);

    log_uart_get()->flush();

    fk::Leds leds;
    leds.notifyFatal();

    // This will happen until the WDT kicks is back to Reset, hopefully. I'm
    // flashing these just to sort of give the user an idea that something's
    // going wrong if they happen to be around.
    while (1) {
        leds.task();
        delay(10);
    }
    #else

    fprintf(stderr, "ASSERTION: %s:%d '%s'", file, lineno, msg);
    abort();

    #endif // defined(ARDUINO)
}
