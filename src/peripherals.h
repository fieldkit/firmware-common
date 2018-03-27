#ifndef FK_PERIPHERALS_H_INCLUDED
#define FK_PERIPHERALS_H_INCLUDED

#include "hardware.h"

namespace fk {

class Peripheral {
private:
    uint32_t acquiredAt{ 0 };
    void *owner{ nullptr };

public:
    bool available() {
        return acquiredAt == 0;
    }

    bool tryAcquire(void *newOwner) {
        if (available()) {
            acquire(newOwner);
            return true;
        }
        return false;
    }

    // TODO: Specify who?
    void acquire(void *newOwner) {
        fk_assert(available());
        acquiredAt = millis();
        owner = newOwner;
    }

    void release(void *anOwner) {
        fk_assert(owner == anOwner);
        fk_assert(!available());
        acquiredAt = 0;
        owner = nullptr;
    }

    bool isOwner(void *anOwner) {
        return owner == anOwner;
    }
};

class Peripherals {
private:
    Peripheral _twoWire1;
    Peripheral _twoWire2;
    Peripheral _twoWire3;

public:
    Peripheral &twoWire1() {
        return _twoWire1;
    }

    Peripheral &twoWire2() {
        return _twoWire2;
    }

    Peripheral &twoWire3() {
        return _twoWire3;
    }

};

extern Peripherals peripherals;

}

#endif
