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

SendDataToLoraGateway::SendDataToLoraGateway(RadioService &radioService, FileSystem &fileSystem, FileCopySettings settings) :
    Task("SendDataToLoraGateway"), radioService(&radioService), fileSystem(&fileSystem), settings(settings) {
}

void SendDataToLoraGateway::enqueued() {
    log("Enqueued!");
    started = false;
}

TaskEval SendDataToLoraGateway::task() {
    if (!radioService->isAvailable()) {
        log("No radio.");
        return TaskEval::done();
    }

    if (!started) {
        started = true;
        copying = true;
        if (!fileSystem->beginFileCopy(settings)) {
            log("Failed to open file for reading.");
            return TaskEval::error();
        }

        auto &fileCopy = fileSystem->files().fileCopy();
        if (fileCopy.size() == 0) {
            log("Empty file");
            return TaskEval::done();
        }

        if (fileCopy.size() < RadioTransmitFileMaximumSize) {
            radioService->sendToGateway(fileCopy.size());
            log("Beginning, opened file (%d bytes).", fileCopy.size());
        }
        else {
            log("Opened file, too large (%d bytes).", fileCopy.size());
            return TaskEval::done();
        }
    }

    if (radioService->hasErrorOccured()) {
        return TaskEval::error();
    }

    if (radioService->isSleeping()) {
        return TaskEval::done();
    }

    if (copying) {
        auto &fileCopy = fileSystem->files().fileCopy();
        auto &writer = radioService->getWriter();
        if (!fileCopy.tick(writer)) {
            log("Done!");
            writer.close();
            copying = false;
        }
    }

    return TaskEval::busy();
}

#endif

}
