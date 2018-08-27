#ifndef FK_MODULE_STARTUP_H_INCLUDED
#define FK_MODULE_STARTUP_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class Booting : public ModuleServicesState {
public:
    const char *name() const override {
        return "Booting";
    }

public:
    void task() override;

private:
    void setupFlash();

};

}

#endif
