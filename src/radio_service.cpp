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

    available_ = true;

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
    Task("SendDataToLoraGateway"), radioService(&radioService), files(&fileSystem.files()) {
}

void SendDataToLoraGateway::enqueued() {
    log("Enqueued!");
    started = false;
}

TaskEval SendDataToLoraGateway::task() {
    auto &fileReader = files->reader();
    if (!radioService->isAvailable()) {
        log("No radio.");
        return TaskEval::done();
    }
    if (!started) {
        started = true;
        copying = true;
        fileReader.open();
        streamCopier.restart();
        if (fileReader.size() < RadioTransmitFileMaximumSize) {
            radioService->sendToGateway(fileReader.size());
            log("Beginning, opened file (%d bytes).", fileReader.size());
        }
        else {
            log("Opened file, too large (%d bytes).", fileReader.size());
            return TaskEval::done();
        }
    }

    if (radioService->hasErrorOccured()) {
        return TaskEval::error();
    }
    if (radioService->isSleeping()) {
        if (RadioTransmitFileTruncateAfter) {
            log("Truncating data!");
            fileReader.truncate();
        }
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
