#ifndef FK_TRANSMIT_LORA_DATA_H_INCLUDED
#define FK_TRANSMIT_LORA_DATA_H_INCLUDED

#include "state_services.h"

namespace fk {

class TransmitLoraData : public MainServicesState {
public:
    TransmitLoraData() {
    }

public:
    const char *name() const override {
        return "TransmitLoraData";
    }

public:
    void task() override;
};

}

#endif
