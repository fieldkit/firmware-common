#ifndef FK_BATTERY_GAUGE_H_INCLUDED
#define FK_BATTERY_GAUGE_H_INCLUDED

#include <cinttypes>

namespace fk {

class BatteryGauge {
public:
    struct BatteryReading {
        bool charging;
        float voltage;
        float ma;
        float coulombs;
        uint16_t counter;

        bool reliable() const;
    };

public:
    bool available();
    bool enable();
    bool disable();

public:
    BatteryReading read();

};

}

#endif
