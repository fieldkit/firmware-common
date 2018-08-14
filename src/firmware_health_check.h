#ifndef FK_FIRMWARE_HEALTH_CHECK_H_INCLUDED
#define FK_FIRMWARE_HEALTH_CHECK_H_INCLUDED

#include "state_services.h"

namespace fk {

class FirmwareHealthCheck : public MainServicesState {
public:
    const char *name() const override {
        return "FirmwareHealthCheck";
    }

public:
    void task() override;
};

}

#endif
