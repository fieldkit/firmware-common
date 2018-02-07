#ifndef FK_WIFI_CONNECTIONS_H_INCLUDED
#define FK_WIFI_CONNECTIONS_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "network_settings.h"
#include "wifi_message_buffer.h"

namespace fk {

class ReadAppQuery : public Task {
private:
    uint32_t dieAt{ 0 };
    WiFiClient *wcl;
    AppServicer *servicer;
    WifiMessageBuffer *buffer;

public:
    ReadAppQuery(WiFiClient &wcl, AppServicer &servicer, WifiMessageBuffer &buffer);

public:
    void enqueued() override {
        dieAt = 0;
    }
    TaskEval task() override;
};

class HandleConnection : public ActiveObject {
private:
    AppServicer *servicer;
    WiFiClient wcl;
    WifiMessageBuffer buffer;
    ReadAppQuery readAppQuery;

public:
    HandleConnection(AppServicer &servicer);

    void setConnection(WiFiClient &newClient) {
        wcl = newClient;
        buffer.setConnection(wcl);
    }

public:
    void enqueued() override;
    void done() override;
};

enum class ListenerState {
    Idle,
    Disconnected,
    Listening,
    Busy,
};

class Listen : public Task {
private:
    uint32_t lastActivity{ 0 };
    ListenerState state{ ListenerState::Idle };
    Pool pool;
    WiFiServer server;
    AppServicer *servicer;
    HandleConnection handleConnection;

public:
    Listen(uint16_t port, AppServicer &servicer);

public:
    void begin();
    void end();
    bool inactive();
    bool hasConnection() {
        return state == ListenerState::Busy;
    }

public:
    TaskEval task() override;
};

}

#endif
