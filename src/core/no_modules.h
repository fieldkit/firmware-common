#ifndef FK_NO_MODULES_H_INCLUDED
#define FK_NO_MODULES_H_INCLUDED

#include "state_services.h"

namespace fk {

class NoModulesThrottle : public MainServicesState {
private:
    uint32_t entered_{ 0 };

public:
    const char *name() const override {
        return "NoModulesThrottle";
    }

public:
    void entry() override;
    void task() override;
};

}

#endif
