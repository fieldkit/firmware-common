#ifndef FK_FACTORY_RESET_CHECK_H_INCLUDED
#define FK_FACTORY_RESET_CHECK_H_INCLUDED

#include "state_services.h"

namespace fk {

class FactoryResetCheck : public MainServicesState {
public:
    const char *name() const override {
        return "FactoryResetCheck";
    }

public:
    void task() override;
    void react(ShortButtonPressEvent const &sbpe);
    void react(LongButtonPressEvent const &lbpe);

};

}

#endif
