#ifndef FK_WIFI_LIVE_DATA_H_INCLUDED
#define FK_WIFI_LIVE_DATA_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class WifiLiveData : public WifiState {
private:
    uint32_t interval_{ 0 };
    uint32_t lastReadings_{ 0 };
    uint32_t lastPolled_{ 0 };

public:
    WifiLiveData() {
    }

    WifiLiveData(uint32_t interval) : interval_(interval) {
    }

public:
    const char *name() const override {
        return "WifiLiveData";
    }

public:
    void react(LiveDataEvent const &lde) override;
    void react(AppQueryEvent const &aqe) override;
    void entry() override;
    void task() override;
};

}

#endif
