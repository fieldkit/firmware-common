#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "wifi_connections.h"
#include "core_state.h"
#include "app_servicer.h"

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
    CreateWifiAp(CoreState &state) : Task("ConnectWifiAp"), state(&state) {
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
    CoreState *state;
    ConnectToWifiAp connectToWifiAp;
    CreateWifiAp createWifiAp;
    Delay delay{ 5000 };
    Listen listen;

public:
    Wifi(CoreState &state, AppServicer &servicer);

public:
    void begin();
    void done(Task &task) override;
    void error(Task &task) override;
    void idle() override;

private:
    bool isListening();
    bool isDisconnected();
    void ensureDisconnected();
    bool readyToServe();

};

}

#endif
