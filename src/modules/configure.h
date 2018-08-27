#ifndef FK_CONFIGURE_MODULE_H_INCLUDED
#define FK_CONFIGURE_MODULE_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class ConfigureModule : public ModuleServicesState {
public:
    const char *name() const override {
        return "ConfigureModule";
    }

public:
    void task() override;
};

}

#endif
