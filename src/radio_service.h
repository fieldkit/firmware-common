#ifndef FK_RADIO_SERVICE_H_INCLUDED
#define FK_RADIO_SERVICE_H_INCLUDED

#ifdef FK_ENABLE_RADIO

#include <lora_radio_rh.h>
#include <node_protocol.h>

#include "active_object.h"
#include "device_id.h"
#include "file_system.h"
#include "file_reader.h"

namespace fk {

class RadioService;

class SendDataToLoraGateway : public Task {
private:
    RadioService *radioService;
    FileReader fileReader;
    lws::AlignedStorageBuffer<128> buffer;
    lws::StreamCopier streamCopier;
    bool started{ false };
    bool copying{ false };

public:
    SendDataToLoraGateway(RadioService &radioService, FileSystem &fileSystem, uint8_t file);

public:
    void enqueued() override;
    TaskEval task() override;

};

class RadioService : public Task, NodeNetworkCallbacks {
private:
    LoraRadioRadioHead radio;
    NodeNetworkProtocol protocol;
    lws::CircularStreams<lws::RingBufferN<256>> outgoing;

public:
    RadioService();

public:
    bool setup(DeviceId &deviceId);
    void sendToGateway();
    bool hasErrorOccured() {
        return protocol.hasErrorOccured();
    }
    bool isSleeping() {
        return protocol.isSleeping();
    }

public:
    lws::Writer &getWriter() {
        return outgoing.getWriter();
    }
    lws::Reader *openReader() override;
    void closeReader(lws::Reader *reader) override;

public:
    TaskEval task() override;

};

}

#endif

#endif