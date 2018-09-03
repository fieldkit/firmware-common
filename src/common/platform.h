#ifndef FK_PLATFORM_H_INCLUDED
#define FK_PLATFORM_H_INCLUDED

#include <cinttypes>

// Arduino
class Uart;
class Print;

namespace fk {

uint32_t fk_uptime();

uint32_t fk_uptime_adjust(uint32_t by);

bool fk_console_attached();

bool fk_logging_trace_enabled();

uint32_t fk_wdt_period(uint32_t ms);

uint32_t fk_wdt_enable();

bool fk_wdt_early_warning_read();

void fk_wdt_early_warning_clear();

void fk_wdt_checkin();

void fk_system_sleep();

void fk_system_reset();

void fk_delay(uint32_t ms);

bool fk_system_reset_cause_is_wdt();

uint8_t fk_system_reset_cause_get();

const char *fk_system_reset_cause_get_string();

}

#endif
