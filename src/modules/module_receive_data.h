#ifndef FK_MODULE_RECEIVE_DATA_H_INCLUDED
#define FK_MODULE_RECEIVE_DATA_H_INCLUDED

#include "module_fsm.h"
#include "module_copy_settings.h"

namespace fk {

class ModuleReceiveData : public ModuleServicesState {
private:
    ModuleCopySettings settings_;

public:
    ModuleReceiveData();

    ModuleReceiveData(ModuleCopySettings settings);

public:
    const char *name() const override {
        return "ModuleReceiveData";
    }

public:
    void task() override;
};

}

#endif
