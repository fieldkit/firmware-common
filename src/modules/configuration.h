#ifndef FK_MODULE_CONFIGURATION_H_INCLUDED
#define FK_MODULE_CONFIGURATION_H_INCLUDED

#include "common_configuration.h"

namespace fk {

struct Configuration {
    CommonConfiguration common;
};

extern const Configuration configuration;

}

#endif
