#include "radio_service.h"
#include "hardware.h"

namespace fk {

#ifdef FK_ENABLE_RADIO

RadioService::RadioService() : Task("Radio"), radio{ Hardware::RFM95_PIN_CS, Hardware::RFM95_PIN_D0, Hardware::RFM95_PIN_ENABLE, Hardware::RFM95_PIN_RESET }, protocol{ radio, *this } {
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

void RadioService::sendToGateway(size_t size) {
    log("Cleared buffers.");
    outgoing.clear();
    protocol.sendToGateway();
    size_ = size;
}

NodeNetworkCallbacks::OpenedReader RadioService::openReader() {
    return NodeNetworkCallbacks::OpenedReader{ &outgoing.getReader(), size_ };
}

void RadioService::closeReader(lws::Reader *reader) {
}

TaskEval RadioService::task() {
    protocol.tick();

    if (radio.hasPacket()) {
        auto lora = radio.getLoraPacket();
        protocol.push(lora);
    }

    return TaskEval::busy();
}

SendDataToLoraGateway::SendDataToLoraGateway(RadioService &radioService, FileSystem &fileSystem, uint8_t file) :
    Task("SendDataToLoraGateway"), radioService(&radioService), fileReader(fileSystem, file) {
}

void SendDataToLoraGateway::enqueued() {
    log("Enqueued!");
    started = false;
}

TaskEval SendDataToLoraGateway::task() {
    if (!started) {
        started = true;
        copying = true;
        fileReader.open();
        streamCopier.restart();
        radioService->sendToGateway(fileReader.size());
        log("Beginning, opened file (%d bytes).", fileReader.size());
    }

    if (radioService->hasErrorOccured()) {
        return TaskEval::error();
    }
    if (radioService->isSleeping()) {
        log("Truncating data!");
        fileReader.truncate();
        return TaskEval::done();
    }

    if (copying) {
        auto& writer = radioService->getWriter();
        while (true) {
            auto bytes = streamCopier.copy(fileReader, writer);
            if (bytes == lws::Stream::EOS) {
                log("Done!");
                writer.close();
                copying = false;
                break;
            }
            if (bytes == 0) {
                break;
            }
        }
    }

    return TaskEval::busy();
}

#endif

}
