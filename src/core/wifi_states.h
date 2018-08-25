#ifndef FK_WIFI_STATES_H_INCLUDED
#define FK_WIFI_STATES_H_INCLUDED

#include "state_services.h"

namespace fk {

class WifiState : public WifiServicesState {
public:
    void serve();

};

}

#endif
