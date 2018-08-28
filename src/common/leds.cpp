#include <Arduino.h>

#include "leds.h"
#include "tuning.h"
#include "platform.h"

namespace fk {

Leds::Leds() {
}

void Leds::setup() {
    // Note that at least one board right now uses 13 for other purposes so
    // ths should be done before that happens.
    #if !defined(FK_HARDWARE_WIRE11AND13_ENABLE)
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    #endif
    // I removed the above always happening because of a change that occured in the Arduino Core.
    // Basically the call to pinMode no longer disables pullups or something (I
    // need to read up more) and so this was breaking the use of pin 13 for I2C.
    // https://github.com/arduino/ArduinoCore-samd/commit/33efce53f509e276f8c7e727ab425ed7427e9bfd

    pinMode(A3, OUTPUT);
}

void Leds::task() {
}

bool Leds::disabled() {
    if (LedsDisableAfter == 0) {
        return false;
    }
    return fk_uptime() > LedsDisableAfter;
}

void Leds::notifyAlive() {
}

void Leds::notifyBattery(float percentage) {
}

void Leds::notifyNoModules() {
}

void Leds::notifyReadingsBegin() {
}

void Leds::notifyReadingsDone() {
}

void Leds::notifyFatal() {
}

}
