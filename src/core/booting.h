#ifndef FK_BOOTING_H_INCLUDED
#define FK_BOOTING_H_INCLUDED

#include "state_services.h"

namespace fk {

class Booting : public MainServicesState {
public:
    const char *name() const override {
        return "Booting";
    }

public:
    void task() override;

};

}

#endif
