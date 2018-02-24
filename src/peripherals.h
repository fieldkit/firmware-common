#ifndef FK_PERIPHERALS_H_INCLUDED
#define FK_PERIPHERALS_H_INCLUDED

namespace fk {

class Peripheral {
private:
    uint32_t acquiredAt{ 0 };

public:
    bool available() {
        return acquiredAt == 0;
    }

    bool tryAcquire() {
        if (available()) {
            acquire();
            return true;
        }
        return false;
    }

    // TODO: Specify who?
    void acquire() {
        fk_assert(available());
        acquiredAt = millis();
    }

    void release() {
        fk_assert(!available());
        acquiredAt = 0;
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
