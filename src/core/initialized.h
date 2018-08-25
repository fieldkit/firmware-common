#ifndef FK_INITIALIZED_H_INCLUDED
#define FK_INITIALIZED_H_INCLUDED

#include "state_services.h"

namespace fk {

class Initialized : public CoreDevice {
public:
    const char *name() const override {
        return "Initialized";
    }

public:
    void task() override;
};

}

#endif
