#ifndef FK_TAKE_READINGS_H_INCLUDED
#define FK_TAKE_READINGS_H_INCLUDED

#include "state_services.h"

namespace fk {

class TakeReadings : public MainServicesState {
private:
    uint8_t remaining_{ 1 };

public:
    const char *name() const override {
        return "TakeReadings";
    }

public:
    void entry() override;
    void task() override;

};

}

#endif
