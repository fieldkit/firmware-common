#ifndef FK_WATCHDOG_H_INCLUDED
#define FK_WATCHDOG_H_INCLUDED

#include <cstdint>

namespace fk {

class Watchdog {
private:
    uint32_t time { 0 };

public:
    void tick();

};

}

#endif
