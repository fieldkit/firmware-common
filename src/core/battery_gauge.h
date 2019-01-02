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

        int16_t voltage_s16;
        int16_t current_s16;
        int16_t charge_s16;
        int16_t counter_s16;
    };

public:
    bool available();
    bool enable();
    bool disable();

public:
    BatteryReading read();

private:
    union data16_t {
        uint8_t bytes[2];
        uint16_t u16;
    };

};

}

#endif
