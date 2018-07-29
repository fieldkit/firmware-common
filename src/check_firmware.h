#ifndef FK_CHECK_FIRMWARE_H_INCLUDED
#define FK_CHECK_FIRMWARE_H_INCLUDED

#include "wifi_states.h"

namespace fk {

class CheckFirmware : public WifiState {
public:
    const char *name() const override {
        return "CheckFirmware";
    }

public:
    void task() override;

};

}

#endif
