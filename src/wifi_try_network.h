#ifndef FK_WIFI_TRY_NETWORK_H_INCLUDED
#define FK_WIFI_TRY_NETWORK_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiTryNetwork : public WifiState {
private:
    uint8_t index_;

public:
    WifiTryNetwork() {
    }

    WifiTryNetwork(uint8_t index) : index_(index) {
    }

public:
    const char *name() const override {
        return "WifiTryNetwork";
    }

public:
    void task() override;
};

}

#endif
