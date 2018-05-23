#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "wifi_connections.h"
#include "wifi_server.h"
#include "core_state.h"
#include "wifi_client.h"

namespace fk {

class Wifi : public ActiveObject {
private:
    CoreState *state;
    WifiConnection *connection;
    uint32_t version{ 0 };
    uint8_t status{ WL_IDLE_STATUS };
    uint32_t lastActivityAt{ 0 };
    uint32_t lastStatusAt{ 0 };
    ConnectToWifiAp connectToWifiAp;
    CreateWifiAp createWifiAp;
    ScanNetworks scanNetworks;
    Delay delay{ 5000 };
    Listen listen;
    bool disabled{ false };
    bool busy{ false };
    bool triedAp{ false };

public:
    Wifi(CoreState &state, WifiConnection &connection, AppServicer &servicer, TaskQueue &taskQueue);

public:
    void disable();
    void setBusy(bool newBusy) {
        busy = newBusy;
        lastActivityAt = millis();
    }
    bool isDisabled() {
        return disabled;
    }
    bool possiblyOnline() {
        return !isDisabled() && (WiFi.status() == WL_CONNECTED);
    }
    bool discoveryEnabled() {
        return !isDisabled() && (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_CONNECTED);
    }
    void begin();
    void done(Task &task) override;
    void error(Task &task) override;
    void idle() override;

private:
    bool isListening();
    bool readyToServe();
    void traceStatus();

};

}

#endif
