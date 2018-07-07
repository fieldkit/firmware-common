#ifndef FK_WIFI_STATES_H_INCLUDED
#define FK_WIFI_STATES_H_INCLUDED

#include "core_fsm_states.h"

namespace fk {

class WifiStartup : public WifiServicesState {
private:
    bool initialized_{ false };

public:
    void entry() override;
    void task() override;

};

}

#endif
