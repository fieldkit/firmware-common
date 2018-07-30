#ifndef FK_WIFI_STARTUP_H_INCLUDED
#define FK_WIFI_STARTUP_H_INCLUDED

#include "state_services.h"

namespace fk {

class WifiStartup : public WifiServicesState {
public:
    const char *name() const override {
        return "WifiStartup";
    }

public:
    void task() override;

};

}

#endif
