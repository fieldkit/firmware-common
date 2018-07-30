#ifndef FK_REBOOT_DEVICE_H_INCLUDED
#define FK_REBOOT_DEVICE_H_INCLUDED

#include "state_services.h"

namespace fk {

class RebootDevice : public MainServicesState {
public:
    const char *name() const override {
        return "RebootDevice";
    }

public:
    void task() override;
};

}

#endif
