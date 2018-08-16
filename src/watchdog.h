#ifndef FK_WATCHDOG_H_INCLUDED
#define FK_WATCHDOG_H_INCLUDED

#include <cinttypes>

#include "leds.h"
#include "platform.h"

namespace fk {

class Watchdog : public Task {
private:
    uint32_t idledAt_{ 0 };
    Leds *leds_;

public:
    uint32_t elapsedSinceIdle() const {
        return fk_uptime() - idledAt_;
    }

public:
    Watchdog(Leds &leds) : Task("Watchdog"), leds_(&leds) {
    }

public:
    void setup();
    void started();
    void idling();
    uint32_t sleep(uint32_t ms);

public:
    TaskEval task() override;

};

}

#endif
