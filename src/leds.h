#ifndef LEDS_H_INCLUDED
#define  LEDS_H_INCLUDED

#include "active_object.h"

namespace fk {

enum class LedStatus {
    None,
    NoAttachedModules,
    Fatal,
};

class Leds : public ActiveObject {
private:
    LedStatus status{ LedStatus::None };
    uint32_t nextChange{ 0 };

public:
    Leds();

public:
    void idle() override;

public:
    void setup();
    void all(bool value);
    void on();
    void off();

public:
    void alive();
    void clear();
    void fatal();
    void noAttachedModules();

};

}

#endif
