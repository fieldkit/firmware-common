#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "wifi_connections.h"
#include "core_state.h"
#include "wifi_client.h"

namespace fk {

class ConnectToWifiAp : public Task {
private:
    CoreState *state;
    size_t networkNumber{ 0 };

public:
    ConnectToWifiAp(CoreState &state) : Task("ConnectWifiAp"), state(&state) {
    }

public:
    TaskEval task() override;

};

class CreateWifiAp : public Task {
private:
    CoreState *state;

public:
    CreateWifiAp(CoreState &state) : Task("CreateWifiAp"), state(&state) {
    }

public:
    TaskEval task() override;

};

class ScanNetworks : public Task {
private:
    uint32_t begunAt{ 0 };

public:
    ScanNetworks() : Task("ScanNetworks") {
    }

public:
    TaskEval task() override;

};

class Wifi : public ActiveObject {
public:
    static constexpr uint16_t ServerPort = 54321;

private:
    uint32_t version{ 0 };
    uint8_t status{ WL_IDLE_STATUS };
    uint32_t lastActivityAt{ 0 };
    CoreState *state;
    ConnectToWifiAp connectToWifiAp;
    CreateWifiAp createWifiAp;
    ScanNetworks scanNetworks;
    WifiConnection connection;
    Delay delay{ 5000 };
    Listen listen;
    bool disabled{ false };
    bool busy{ false };

public:
    Wifi(CoreState &state, AppServicer &servicer, TaskQueue &taskQueue);

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
        return !isDisabled() && WiFi.status() == WL_CONNECTED;
    }
    void begin();
    void done(Task &task) override;
    void error(Task &task) override;
    void idle() override;

private:
    bool isListening();
    void ensureDisconnected();
    bool readyToServe();

};

}

#endif
