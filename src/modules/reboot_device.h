#ifndef FK_REBOOT_DEVICE_H_INCLUDED
#define FK_REBOOT_DEVICE_H_INCLUDED

#include "module_fsm.h"

namespace fk {

class RebootDevice : public ModuleServicesState {
public:
    const char *name() const override {
        return "RebootDevice";
    }

public:
    void task() override;
};

}

#endif
