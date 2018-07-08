#ifndef FK_PLATFORM_H_INCLUDED
#define FK_PLATFORM_H_INCLUDED

#include <cinttypes>

// Arduino
class Uart;

namespace fk {

uint32_t fk_uptime();

uint32_t fk_uptime_adjust(uint32_t by);

bool fk_console_attached();

}

#endif
