#ifndef FK_COMMON_CONFIGURATION_H_INCLUDED
#define FK_COMMON_CONFIGURATION_H_INCLUDED

#include "tuning.h"

namespace fk {

struct CommonConfiguration {
    struct Leds {
        /**
         *
         */
        #if defined(FK_NATURALIST)
        uint32_t disable_after{ 0 };
        #else
        uint32_t disable_after{ 60 * Minutes };
        #endif

        /**
         *
         */
        #if defined(FK_NATURALIST)
        uint32_t brightness{ 0 };
        #else
        uint32_t brightness{ 32 };
        #endif
    };

    Leds leds;

    uint32_t require_idle_every{ 2 * Hours };

};

}

#endif
