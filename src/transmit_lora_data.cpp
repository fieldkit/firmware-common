#include "transmit_lora_data.h"

#include "radio_service.h"
#include "wifi.h"

namespace fk {

void TransmitLoraData::task() {
    if (!services().radio->isAvailable()) {
        log("Skip, Radio unavailable.");
        back();
        return;
    }

    if (Wifi::discoveryEnabled()) {
        log("Skip, wifi enabled.");
        back();
        return;
    }

    SendDataToLoraGateway sendDataToLoraGateway{ *services().radio, *services().fileSystem, { FileNumber::Data } };

    sendDataToLoraGateway.enqueued();

    while (simple_task_run(sendDataToLoraGateway)) {
        services().watchdog->task();
        services().radio->task();
    }

    back();
}

}
