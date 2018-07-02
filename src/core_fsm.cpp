#include "core_fsm.h"

namespace fk {

class Idle : public CoreDevice {
    void entry() override {
    }
};

class Initializing : public CoreDevice {
    void entry() override {
        transit<Idle>();
    }
};

class Booting : public CoreDevice {
    void entry() override {
        transit<Initializing>();
    }
};

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
