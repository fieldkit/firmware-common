#ifndef FK_RADIO_SERVICE_H_INCLUDED
#define FK_RADIO_SERVICE_H_INCLUDED

#ifdef FK_ENABLE_RADIO

#include <lora_radio_rh.h>

#include "active_object.h"

namespace fk {

class RadioService : public Task {
private:
    LoraRadioRadioHead radio;

public:
    RadioService();

public:
    bool setup();

    TaskEval task() override;

};

}

#endif

#endif
