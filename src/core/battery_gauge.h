#ifndef FK_BATTERY_GAUGE_H_INCLUDED
#define FK_BATTERY_GAUGE_H_INCLUDED

#include <cinttypes>

namespace fk {

class BatteryGauge {
public:
    struct BatteryReading {
        float voltage;
        float current;
        float charge;
        uint16_t counter;
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
