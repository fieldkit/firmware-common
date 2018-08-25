#ifndef FK_WIFI_DISABLE_H_INCLUDED
#define FK_WIFI_DISABLE_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiDisable : public WifiState {
public:
    const char *name() const override {
        return "WifiDisable";
    }

public:
    void task() override;

};

}

#endif
