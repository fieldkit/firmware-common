#ifndef FK_WIFI_CONNECTION_COMPLETED_H_INCLUDED
#define FK_WIFI_CONNECTION_COMPLETED_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiConnectionCompleted : public WifiState {
public:
    const char *name() const override {
        return "WifiConnectionCompleted";
    }

public:
    void task() override;
};

}

#endif
