#ifndef FK_WATCHDOG_H_INCLUDED
#define FK_WATCHDOG_H_INCLUDED

#include <cstdint>

#include "leds.h"

namespace fk {

class Watchdog : public ActiveObject {
private:
    Leds *leds;

public:
    Watchdog(Leds &leds) : leds(&leds) {
    }

    void setup();

public:
    void idle() override;

};

}

#endif
