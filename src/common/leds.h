#ifndef LEDS_H_INCLUDED
#define LEDS_H_INCLUDED

#include "task.h"

namespace fk {

class Leds {
private:
    uint32_t aliveOff_{ 0 };

public:
    Leds();

public:
    void setup();
    void task();

public:
    void notifyAlive();
    void notifyBattery(float percentage);
    void notifyNoModules();
    void notifyReadingsBegin();
    void notifyReadingsDone();
    void notifyFatal();

private:
    bool disabled();

};

}

#endif
