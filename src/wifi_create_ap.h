#ifndef FK_WIFI_CREATE_AP_H_INCLUDED
#define FK_WIFI_CREATE_AP_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiCreateAp : public WifiState {
public:
    const char *name() const override {
        return "WifiCreateAp";
    }

public:
    void task() override;
};

}

#endif
