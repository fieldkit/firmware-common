#include "radio_service.h"
#include "hardware.h"

namespace fk {

#ifdef FK_ENABLE_RADIO

RadioService::RadioService() : Task("Radio"), radio{ Hardware::RFM95_PIN_CS, Hardware::RFM95_PIN_D0, Hardware::RFM95_PIN_ENABLE, Hardware::RFM95_PIN_RESET }, protocol{ radio } {
}

bool RadioService::setup(DeviceId &deviceId) {
    protocol.setNodeId(NodeLoraId{ deviceId.toBufferPtr() });

    if (!radio.setup()) {
        return false;
    }

    radio.setHeaderTo(0xff);
    radio.setHeaderFrom(0x00);
    radio.setThisAddress(0x00);

    return true;
}

void RadioService::sendToGateway() {
    protocol.sendToGateway();
}

TaskEval RadioService::task() {
    protocol.tick();

    if (radio.hasPacket()) {
        auto lora = radio.getLoraPacket();
        protocol.push(lora);
    }

    return TaskEval::busy();
}

SendDataToLoraGateway::SendDataToLoraGateway(RadioService &radioService) : Task("SendDataToLoraGateway"), radioService(&radioService) {
}

TaskEval SendDataToLoraGateway::task() {
    radioService->sendToGateway();

    return TaskEval::done();
}

#endif

}
