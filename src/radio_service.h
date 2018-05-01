#ifndef FK_RADIO_SERVICE_H_INCLUDED
#define FK_RADIO_SERVICE_H_INCLUDED

#ifdef FK_ENABLE_RADIO

#include <lora_radio_rh.h>
#include <node_protocol.h>

#include "active_object.h"
#include "device_id.h"

namespace fk {

class RadioService;

class SendDataToLoraGateway : public Task {
private:
    RadioService *radioService;

public:
    SendDataToLoraGateway(RadioService &radioService);

public:
    TaskEval task() override;

};

class RadioService : public Task, NodeNetworkCallbacks {
private:
    lws::CountingReader reader { 4096 };
    LoraRadioRadioHead radio;
    NodeNetworkProtocol protocol;

public:
    RadioService();

public:
    bool setup(DeviceId &deviceId);
    void sendToGateway();

public:
    lws::Reader *openReader() override {
        reader = lws::CountingReader(4096);
        return &reader;
    }

    void closeReader(lws::Reader *reader) override {
    }

public:
    TaskEval task() override;

};

}

#endif

#endif
