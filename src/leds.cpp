#include <Arduino.h>

#include "leds.h"

namespace fk {

Leds::Leds() {
}

void Leds::idle() {
    switch (status) {
    case LedStatus::Fatal:  {
        if (nextChange < millis()) {
            // Simple toggle.
            all(!digitalRead(A1));
            nextChange = millis() + 100;
        }
        break;
    }
    default: {
        break;
    }
    }
}

void Leds::setup() {
    // Note that at least one board right now uses 13 for other purposes so
    // ths should be done before that happens.
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    pinMode(A3, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);
    off();
}

void Leds::all(bool value) {
    digitalWrite(A3, value);
    digitalWrite(A4, value);
    digitalWrite(A5, value);
}

void Leds::on() {
    all(HIGH);
}

void Leds::off() {
    all(LOW);
}

void Leds::clear() {
    status = LedStatus::None;
    nextChange = 0;
}

void Leds::alive() {
    digitalWrite(A5, HIGH);
    delay(100);
    digitalWrite(A5, LOW);
}

void Leds::fatal() {
    status = LedStatus::Fatal;
    nextChange = 0;
}

}
