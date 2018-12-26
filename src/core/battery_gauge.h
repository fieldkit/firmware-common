#ifndef FK_BATTERY_GAUGE_H_INCLUDED
#define FK_BATTERY_GAUGE_H_INCLUDED

#include <cinttypes>

namespace fk {

class BatteryGauge {
public:
    bool available();
    bool enable();
    bool disable();

public:
    float current();
    float voltage();
    float stateOfCharge();

private:
    union data16_t {
        uint8_t bytes[2];
        uint16_t u16;
    };
    union data32_t {
        uint8_t bytes[4];
        uint16_t u16[2];
        uint32_t u32;
    };

    bool read(uint8_t reg, data16_t &data);
    bool read(uint8_t reg, data32_t &data);

};

}

#endif
