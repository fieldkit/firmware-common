#ifndef FK_WIFI_SERVER_H_INCLUDED
#define FK_WIFI_SERVER_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "network_settings.h"
#include "wifi_client.h"

namespace fk {

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

public:
    Listen(uint16_t port, AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue);

public:
    void end();
    bool inactive();

private:
    void begin();

public:
    TaskEval task() override;
};

}

#endif
