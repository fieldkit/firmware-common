#ifndef FK_CHECK_POWER_H_INCLUDED
#define FK_CHECK_POWER_H_INCLUDED

#include "core_fsm_states.h"

namespace fk {

class CheckPower : public MainServicesState {
private:
    bool visited_{ false };

public:
    const char *name() const override {
        return "CheckPower";
    }

public:
    void task() override;
};

}

#endif
