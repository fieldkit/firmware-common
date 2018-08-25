#ifndef FK_WIFI_SYNC_TIME_H_INCLUDED
#define FK_WIFI_SYNC_TIME_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiSyncTime : public WifiState {
private:
    bool success_{ false };

public:
    const char *name() const override {
        return "WifiSyncTime";
    }

public:
    void task() override;

};

}

#endif
