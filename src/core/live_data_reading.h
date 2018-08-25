#ifndef FK_LIVE_DATA_READING_H_INCLUDED
#define FK_LIVE_DATA_READING_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class LiveDataReading : public WifiState {
public:
    const char *name() const override {
        return "LiveDataReading";
    }

public:
    void task() override;
};

}

#endif
