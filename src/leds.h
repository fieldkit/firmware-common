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
};

class Blinker : public Task {
private:
    BlinkerKind kind{ BlinkerKind::None };
    uint32_t nextChange{ 0 };

public:
    Blinker(BlinkerKind kind = BlinkerKind::None) : Task("Blinker"), kind(kind) {
    }

public:
    TaskEval task() override;

public:
    bool isOfKind(BlinkerKind k) {
        return kind == k;
    }

    bool isIdle() {
        return isOfKind(BlinkerKind::None);
    }

    void clear() {
        kind = BlinkerKind::None;
    }

};

class Leds : public ActiveObject {
    static constexpr uint8_t MaximumBlinkers = 3;

private:
    Blinker blinkers[MaximumBlinkers];

public:
    Leds();

public:
    void setup();
    void idle() override;

private:
    void all(bool value);
    void push(BlinkerKind kind);
    void clear(BlinkerKind kind);

public:
    void alive();
    void fatal();
    void beginReading();
    void doneReading();
    void noAttachedModules();

};

}

#endif
