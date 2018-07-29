#ifndef FK_STARTUP_H_INCLUDED
#define FK_STARTUP_H_INCLUDED

#include "core_fsm_states.h"

namespace fk {

class Booting : public MainServicesState {
private:
    // TODO: Default this to Initialized
    static Deferred configure_;

public:
    static void configureState(Deferred configure) {
        configure_ = configure;
    }

public:
    const char *name() const override {
        return "Booting";
    }

public:
    void task() override;

};

}

#endif
