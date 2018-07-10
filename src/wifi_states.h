#ifndef FK_WIFI_STATES_H_INCLUDED
#define FK_WIFI_STATES_H_INCLUDED

#include "core_fsm_states.h"

namespace fk {

class WifiStartup : public WifiServicesState {
private:
    bool initialized_{ false };

public:
    const char *name() const override {
        return "WifiStartup";
    }

public:
    void task() override;

};

class WifiState : public WifiServicesState {
public:
    void serve();

};

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
