#include "transmit_lora_data.h"

#include "radio_service.h"
#include "idle.h"
#include "wifi.h"

namespace fk {

void TransmitLoraData::task() {
    if (!services().radio->isAvailable()) {
        log("Skip, Radio unavailable.");
        transit_into<Idle>();
        return;
    }

    if (Wifi::discoveryEnabled()) {
        log("Skip, wifi enabled.");
        transit_into<Idle>();
        return;
    }

    SendDataToLoraGateway sendDataToLoraGateway{ *services().radio, *services().fileSystem, { FileNumber::Data } };

    sendDataToLoraGateway.enqueued();

    while (simple_task_run(sendDataToLoraGateway)) {
        services().watchdog->task();
        services().radio->task();
    }

    transit_into<Idle>();
}

}
