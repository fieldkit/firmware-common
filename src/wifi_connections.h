#ifndef FK_WIFI_CONNECTIONS_H_INCLUDED
#define FK_WIFI_CONNECTIONS_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "network_settings.h"
#include "wifi_client.h"

namespace fk {

constexpr uint32_t ConnectionTimeout = 5000;
constexpr uint32_t ConnectionMemory = 128;
constexpr uint32_t InactivityTimeout = 60 * 1000 * 1;

class ReadAppQuery : public Task {
private:
    uint32_t dieAt{ 0 };
    WifiConnection *connection;
    AppServicer *servicer;
    TaskQueue *taskQueue;

public:
    ReadAppQuery(WifiConnection &connection, AppServicer &servicer, TaskQueue &taskQueue);

public:
    void enqueued() override {
        dieAt = 0;
    }
    TaskEval task() override;
};

class HandleConnection : public Task {
private:
    AppServicer *servicer;
    WifiConnection *connection;
    ReadAppQuery readAppQuery;

public:
    HandleConnection(AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue);

public:
    TaskEval task() override;
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
    StaticPool<ConnectionMemory> pool{ "WifiService" };
    WiFiServer server;
    AppServicer *servicer;
    WifiConnection *connection;
    TaskQueue *taskQueue;
    HandleConnection handleConnection;

public:
    Listen(uint16_t port, AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue);

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
