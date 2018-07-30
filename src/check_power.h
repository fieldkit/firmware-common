#ifndef FK_CHECK_POWER_H_INCLUDED
#define FK_CHECK_POWER_H_INCLUDED

#include "state_services.h"

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
