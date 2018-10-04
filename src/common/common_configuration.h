#ifndef FK_COMMON_CONFIGURATION_H_INCLUDED
#define FK_COMMON_CONFIGURATION_H_INCLUDED

#include "tuning.h"

namespace fk {

struct CommonConfiguration {
    uint32_t require_idle_every{ 2 * Hours };
};

}

#endif
