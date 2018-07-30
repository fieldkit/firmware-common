#ifndef FK_USER_WAKEUP_H_INCLUDED
#define FK_USER_WAKEUP_H_INCLUDED

#include "state_services.h"

namespace fk {

class UserWakeup : public MainServicesState {
public:
    const char *name() const override {
        return "UserWakeup";
    }

public:
    void task() override;
};

}

#endif
