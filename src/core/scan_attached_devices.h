#ifndef FK_SCAN_ATTACHED_DEVICES_H_INCLUDED
#define FK_SCAN_ATTACHED_DEVICES_H_INCLUDED

#include "attached_devices.h"
#include "state_services.h"

namespace fk {

class ScanAttachedDevices : public MainServicesState {
private:
    bool first_{ true };

public:
    const char *name() const override {
        return "ScanAttachedDevices";
    }

public:
    void task() override;
};

}

#endif
