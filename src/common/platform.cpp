#include <../Adafruit_ASFcore/reset.h>
#include <../Adafruit_ASFcore/power.h>
#undef LOW
#undef HIGH
#include <Arduino.h>
#include <AtSamd.h>

#include "platform.h"
#include "debug.h"

namespace fk {

static uint32_t offset{ 0 };

static bool console_attached{ false };

void fk_delay(uint32_t ms) {
    delay(ms);
}

uint32_t fk_uptime() {
    return millis() + offset;
}

uint32_t fk_uptime_adjust(uint32_t by) {
    offset += by;
    return offset;
}

bool fk_console_attached() {
    if (Serial) {
        console_attached = true;
    }
    return console_attached;
}

bool fk_logging_trace_enabled() {
    return fk_console_attached();
}

uint32_t fk_wdt_period(uint32_t ms) {
    if (ms >= 8192) {
        return 8192;
    }
    return 0;
}

uint32_t fk_wdt_enable() {
    wdt_enable(WDT_PERIOD_8X, false);
    return 8192;
}

bool fk_wdt_early_warning_read() {
    return wdt_read_early_warning();
}

void fk_wdt_early_warning_clear()  {
    wdt_clear_early_warning();
}

void fk_wdt_checkin() {
    wdt_checkin();
}

void fk_system_sleep() {
    system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
    system_sleep();
}

void fk_system_reset() {
    NVIC_SystemReset();
}

bool fk_system_reset_cause_is_wdt() {
    return system_get_reset_cause() == SYSTEM_RESET_CAUSE_WDT;
}

uint8_t fk_system_reset_cause_get() {
    return system_get_reset_cause();
}

const char *fk_system_reset_cause_get_string() {
    switch (system_get_reset_cause()) {
    case SYSTEM_RESET_CAUSE_SOFTWARE: return "Software";
    case SYSTEM_RESET_CAUSE_WDT: return "WDT";
    case SYSTEM_RESET_CAUSE_EXTERNAL_RESET: return "External Reset";
    case SYSTEM_RESET_CAUSE_BOD33: return "BOD33";
    case SYSTEM_RESET_CAUSE_BOD12: return "BOD12";
    case SYSTEM_RESET_CAUSE_POR: return "PoR";
    default: {
        return "Unknown";
    }
    }
}

}

namespace std {

void __throw_bad_alloc() {
    loginfof("Assert", "std::bad_alloc");
    while (true) {
    }
}

}
