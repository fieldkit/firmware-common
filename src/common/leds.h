#ifndef LEDS_H_INCLUDED
#define LEDS_H_INCLUDED

#include <Adafruit_NeoPixel.h>

namespace fk {

class Leds {
private:
    Adafruit_NeoPixel pixel_{ 1, A3, NEO_GRB + NEO_KHZ400 };

public:
    Leds();

public:
    void setup();
    bool task();

public:
    void notifyStarted();
    void notifyAlive();
    void notifyBattery(float percentage);
    void notifyNoModules();
    void notifyReadingsBegin();
    void notifyReadingsDone();
    void notifyFatal();
    void notifyCaution();
    void notifyWarning();
    void notifyHappy();
    void notifyButtonPressed();
    void notifyButtonLong();
    void notifyButtonShort();
    void notifyButtonReleased();
    void off();

private:
    bool disabled();

};

}

#endif
