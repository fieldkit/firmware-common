#ifndef FK_MODULE_IDLE_H_INCLUDED
#define FK_MODULE_IDLE_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class ModuleIdle : public ModuleServicesState {
public:
    const char *name() const override {
        return "ModuleIdle";
    }

public:
    void react(ModuleQueryEvent const &mqe) override;
    void task() override;
};

}

#endif
