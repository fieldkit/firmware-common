#ifndef FK_MODULE_RECEIVE_DATA_H_INCLUDED
#define FK_MODULE_RECEIVE_DATA_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class ModuleReceiveData : public ModuleServicesState {
public:
    const char *name() const override {
        return "ModuleReceiveData";
    }

public:
    void task() override;
};

}

#endif