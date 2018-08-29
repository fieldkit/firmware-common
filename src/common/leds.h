#ifndef LEDS_H_INCLUDED
#define LEDS_H_INCLUDED

#include <Adafruit_NeoPixel.h>

#include "task.h"

namespace fk {

class Leds {
private:
    Adafruit_NeoPixel pixel_{ 1, A3, NEO_GRB + NEO_KHZ400 };
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
    void notifyHappy();

private:
    bool disabled();

};

}

#endif
