#ifndef FK_UPGRADE_MODULE_FIRMWARE_H_INCLUDED
#define FK_UPGRADE_MODULE_FIRMWARE_H_INCLUDED

#include "state_services.h"

namespace fk {

class UpgradeModuleFirmware : public MainServicesState {
public:
    const char *name() const override {
        return "UpgradeModuleFirmware";
    }

public:
    void task() override;
};

}

#endif
