#ifndef FK_BEGIN_GATHER_READINGS_H_INCLUDED
#define FK_BEGIN_GATHER_READINGS_H_INCLUDED

#include "state_services.h"

namespace fk {

class BeginGatherReadings : public MainServicesState {
public:
    const char *name() const override {
        return "BeginGatherReadings";
    }

public:
    void task() override;
};

}

#endif
