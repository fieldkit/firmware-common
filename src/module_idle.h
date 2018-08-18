#ifndef FK_MODULE_IDLE_H_INCLUDED
#define FK_MODULE_IDLE_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class ModuleIdle : public ModuleServicesState {
private:
    uint32_t tick_{ 0 };

public:
    const char *name() const override {
        return "ModuleIdle";
    }

public:
    void entry() override;
    void task() override;
    bool busy() override {
        return false;
    }
};

}

#endif
