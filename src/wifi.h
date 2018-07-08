#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "wifi_server.h"
#include "core_state.h"
#include "wifi_client.h"

namespace fk {

inline bool wifiPossiblyOnline() {
    return WiFi.status() == WL_CONNECTED;
}

inline bool wifiDiscoveryEnabled() {
    return WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_CONNECTED;
}

class Wifi : public Task {
private:
    WifiConnection *connection;
    Listen listen;
    bool disabled{ false };

public:
    Wifi(WifiConnection &connection, AppServicer &servicer);

public:
    bool begin();
    void disable();

    bool possiblyOnline() {
        return !isDisabled() && wifiPossiblyOnline();
    }

    bool discoveryEnabled() {
        return !isDisabled() && wifiDiscoveryEnabled();
    }

    Listen &server() {
        return listen;
    }

    bool active() const {
        return !isDisabled();
    }

public:
    TaskEval task() override {
        return TaskEval::idle();
    }

private:
    bool isDisabled() const {
        return disabled;
    }

};

}

#endif
