#ifndef FK_WATCHDOG_H_INCLUDED
#define FK_WATCHDOG_H_INCLUDED

#include <cinttypes>

#include "leds.h"

namespace fk {

class Watchdog : public Task {
private:
    Leds *leds;

public:
    Watchdog(Leds &leds) : Task("Watchdog"), leds(&leds) {
    }

public:
    void setup();
    void started();

public:
    TaskEval task() override;

public:
    uint32_t sleep(uint32_t ms);

};

}

#endif
