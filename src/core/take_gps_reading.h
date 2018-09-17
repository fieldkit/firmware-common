#ifndef FK_TAKE_GPS_READING_H_INCLUDED
#define FK_TAKE_GPS_READING_H_INCLUDED

#include "state_services.h"

namespace fk {

class TakeGpsReading : public MainServicesState {
public:
    const char *name() const override {
        return "TakeGpsReading";
    }

public:
    void task() override;

};

}

#endif
