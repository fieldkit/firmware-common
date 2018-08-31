#ifndef FK_STARTUP_H_INCLUDED
#define FK_STARTUP_H_INCLUDED

#include "state_services.h"

namespace fk {

class StartSystem : public MainServicesState {
private:
    // TODO: Default this to Initialized
    static Deferred configure_;

public:
    static void configureState(Deferred configure) {
        configure_ = configure;
    }

public:
    const char *name() const override {
        return "StartSystem";
    }

public:
    void task() override;

private:
    void setupFlash();

};

}

#endif
