#ifndef FK_MODULE_FIRMWARE_SELF_FLASH_H_INCLUDED
#define FK_MODULE_FIRMWARE_SELF_FLASH_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class ModuleFirmwareSelfFlash : public ModuleServicesState {
public:
    const char *name() const override {
        return "ModuleFirmwareSelfFlash";
    }

public:
    void task() override;
};

}

#endif
