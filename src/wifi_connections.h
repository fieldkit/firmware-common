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

}

#endif
