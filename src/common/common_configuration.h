#ifndef FK_COMMON_CONFIGURATION_H_INCLUDED
#define FK_COMMON_CONFIGURATION_H_INCLUDED

#include "tuning.h"

namespace fk {

struct CommonConfiguration {
    struct Leds {
        /**
         *
         */
        uint32_t disable_after{ 60 * Minutes };
    };

    Leds leds;

    uint32_t require_idle_every{ 2 * Hours };

};

}

#endif
