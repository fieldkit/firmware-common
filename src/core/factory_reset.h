#ifndef FK_FACTORY_RESET_H_INCLUDED
#define FK_FACTORY_RESET_H_INCLUDED

#include "state_services.h"

namespace fk {

class FactoryReset : public MainServicesState {
public:
    const char *name() const override {
        return "FactoryReset";
    }

public:
    void task() override;

};

}

#endif
