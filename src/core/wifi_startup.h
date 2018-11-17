#ifndef FK_WIFI_STARTUP_H_INCLUDED
#define FK_WIFI_STARTUP_H_INCLUDED

#include "state_services.h"

namespace fk {

class WifiStartup : public WifiServicesState {
private:
    WifiCheckConfig config_;

public:
    const char *name() const override {
        return "WifiStartup";
    }

public:
    WifiStartup();
    WifiStartup(WifiCheckConfig config);

public:
    void task() override;

};

}

#endif
