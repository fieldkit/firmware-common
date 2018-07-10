#ifndef LEDS_H_INCLUDED
#define  LEDS_H_INCLUDED

#include "active_object.h"

namespace fk {

enum class BlinkerKind {
    None,
    Fatal,
    NoAttachedModules,
    Alive,
    Reading,
    Status,
};

class Blinker : public Task {
private:
    BlinkerKind kind{ BlinkerKind::None };
    uint32_t nextChange{ 0 };
    uint8_t blinksRemaining{ 0 };

public:
    Blinker(BlinkerKind kind = BlinkerKind::None, uint8_t blinksRemaining = 0) : Task("Blinker"), kind(kind), blinksRemaining(blinksRemaining) {
    }

public:
    TaskEval task() override;

public:
    void clear();

    bool isOfKind(BlinkerKind k) {
        return kind == k;
    }

    bool isIdle() {
        return isOfKind(BlinkerKind::None);
    }

};

class Leds : public Task {
    static constexpr uint8_t MaximumBlinkers = 3;

private:
    Blinker blinkers[MaximumBlinkers];

public:
    Leds();

public:
    void setup();

public:
    TaskEval task() override;

private:
    void push(BlinkerKind kind, uint8_t blinksRemaining = 0);
    void clear(BlinkerKind kind);

public:
    void all(bool value);
    void restarting();
    void alive();
    void fatal();
    void blink(uint32_t duration);
    void takingReadings();
    void doneTakingReadings();
    void noAttachedModules();
    void haveAttachedModules();
    void status(uint8_t batteryBlinks);

private:
    bool disabled();

};

}

#endif
