#ifndef FK_TURN_OFF_H_INCLUDED
#define FK_TURN_OFF_H_INCLUDED

#include "state_services.h"
#include "wifi_states.h"

namespace fk {

class TurnOff : public WifiState {
public:
    const char *name() const override {
        return "TurnOff";
    }

public:
    void task() override;

public:
    void react(UserButtonEvent const &ignored) override;
    void react(MinorButtonPressEvent const &ignored) override;
    void react(ShortButtonPressEvent const &ignored) override;
    void react(LongButtonPressEvent const &ignored) override;
};

}

#endif
