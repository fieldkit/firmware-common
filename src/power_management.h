#ifndef FK_POWER_H_INCLUDED
#define FK_POWER_H_INCLUDED

#include <cstdint>
#include <FuelGauge.h>

namespace fk {

class Power {
private:
    FuelGauge gauge;
    uint32_t time { 0 };

public:
    void setup();
    void tick();

};

}

#endif
