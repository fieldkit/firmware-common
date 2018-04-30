#ifndef FK_RADIO_SERVICE_H_INCLUDED
#define FK_RADIO_SERVICE_H_INCLUDED

#ifdef FK_ENABLE_RADIO

#include <lora_radio_rh.h>

#include "active_object.h"
#include "device_id.h"

namespace fk {

class RadioService : public Task {
private:
    LoraRadioRadioHead radio;
    NodeNetworkProtocol protocol;

public:
    RadioService();

public:
    bool setup(DeviceId &deviceId);

public:
    TaskEval task() override;

};

}

#endif

#endif
