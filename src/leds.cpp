#include "leds.h"
#include "tuning.h"

namespace fk {

TaskEval Blinker::task() {
    switch (kind) {
    case BlinkerKind::Fatal:  {
        if (nextChange < fk_uptime()) {
            auto value = !digitalRead(A1);
            digitalWrite(A3, value);
            digitalWrite(A4, value);
            digitalWrite(A5, value);
            nextChange = fk_uptime() + 100;
        }
        break;
    }
    case BlinkerKind::NoAttachedModules:  {
        if (nextChange < fk_uptime()) {
            digitalWrite(A3, !digitalRead(A3));
            nextChange = fk_uptime() + 500;
        }
        break;
    }
    case BlinkerKind::Alive:  {
        break;
    }
    case BlinkerKind::Reading:  {
        digitalWrite(A4, HIGH);
        break;
    }
    case BlinkerKind::Status:  {
        if (nextChange < fk_uptime()) {
            if (digitalRead(A5)) {
                digitalWrite(A5, LOW);
                if (blinksRemaining > 1) {
                    blinksRemaining--;
                    nextChange = fk_uptime() + 200;
                }
                else {
                    clear();
                }
            }
            else {
                digitalWrite(A5, HIGH);
                nextChange = fk_uptime() + 200;
            }
        }
        break;
    }
    default: {
        break;
    }
    }

    return TaskEval::idle();
}

void Blinker::clear() {
    switch (kind) {
    case BlinkerKind::Reading:  {
        digitalWrite(A4, LOW);
        break;
    }
    default: {
        break;
    }
    }

    kind = BlinkerKind::None;
}

Leds::Leds() : Task("LEDs") {
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
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);

    all(LOW);
}

TaskEval Leds::task() {
    for (auto i = 0; i < MaximumBlinkers; ++i) {
        blinkers[i].task();
    }
    return TaskEval::busy();
}

void Leds::all(bool value) {
    if (!disabled()) {
        digitalWrite(A3, value);
        digitalWrite(A4, value);
        digitalWrite(A5, value);
    }
}

void Leds::push(BlinkerKind kind, uint8_t blinksRemaining) {
    if (disabled()) {
        return;
    }

    for (auto i = 0; i < MaximumBlinkers; ++i) {
        if (blinkers[i].isIdle()) {
            trace("Blinker #%d is %d", i, kind);
            blinkers[i] = Blinker{ kind, blinksRemaining };
            return;
        }
    }

    warn("No available blinkers for %d", kind);
}

void Leds::clear(BlinkerKind kind) {
    for (auto i = 0; i < MaximumBlinkers; ++i) {
        if (blinkers[i].isOfKind(kind)) {
            trace("Clearing blinker #%d", i);
            blinkers[i].clear();
        }
    }
}

void Leds::alive() {
    if (!disabled()) {
        digitalWrite(A3, HIGH);
        delay(100);
        digitalWrite(A3, LOW);
    }
}

void Leds::restarting() {
    all(true);
}

void Leds::fatal() {
    push(BlinkerKind::Fatal);
}

void Leds::blink(uint32_t duration) {
    all(true);
    delay(duration);
    all(false);
}

void Leds::takingReadings() {
    push(BlinkerKind::Reading);
}

void Leds::doneTakingReadings() {
    clear(BlinkerKind::Reading);
}

void Leds::noAttachedModules() {
    push(BlinkerKind::NoAttachedModules);
}

void Leds::haveAttachedModules() {
    clear(BlinkerKind::NoAttachedModules);
}

void Leds::status(uint8_t batteryBlinks) {
    push(BlinkerKind::Status, batteryBlinks);
}

bool Leds::disabled() {
    if (LedsDisableAfter == 0) {
        return false;
    }
    return fk_uptime() > LedsDisableAfter;
}

}
