#include <Arduino.h>

#include "hardware.h"
#include "debug.h"

namespace fk {

#if defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

Uart &Hardware::gpsUart = Serial2;

#else // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

Uart &Hardware::gpsUart = Serial1;

#endif // defined(FK_NATURALIST) || defined(FK_CORE_GENERATION_2)

}

extern "C" {

void fk_assertion_hook(void) {
    #if defined(FK_CORE_GENERATION_2)
    loginfof("Assert", "Disabling peripherals.");
    pinMode(fk::Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(fk::Hardware::PIN_PERIPH_ENABLE, LOW);
    #endif
}

}
