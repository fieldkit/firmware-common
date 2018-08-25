#ifndef FK_COPY_MODULE_DATA_H_INCLUDED
#define FK_COPY_MODULE_DATA_H_INCLUDED

#include "state_services.h"

namespace fk {

class CopyModuleData : public MainServicesState {
public:
    const char *name() const override {
        return "CopyModuleData";
    }

public:
    void task() override;
};

}

#endif
