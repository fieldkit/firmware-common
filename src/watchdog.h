#ifndef FK_WATCHDOG_H_INCLUDED
#define FK_WATCHDOG_H_INCLUDED

#include <cstdint>

#include "leds.h"

namespace fk {

class Watchdog {
private:
    uint32_t time { 0 };
    Leds leds;

public:
    void setup();
    void tick();

};

}

#endif
