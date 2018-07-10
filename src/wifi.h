#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>

#include "active_object.h"
#include "wifi_server.h"
#include "wifi_client.h"

namespace fk {

inline bool wifiDiscoveryEnabled() {
    return WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_CONNECTED;
}

class Wifi : public Task {
private:
    Listen listen;
    bool disabled{ false };

public:
    Wifi(WifiConnection &connection);

public:
    bool begin();
    void disable();

public:
    Listen &server() {
        return listen;
    }

protected:
    TaskEval task() override {
        return TaskEval::idle();
    }

};

}

#endif
