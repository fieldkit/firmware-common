#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>

#include "wifi_server.h"
#include "wifi_client.h"

namespace fk {

class Wifi {
private:
    bool initialized_{ false };
    bool disabled_{ true };
    bool available_{ false };
    Listen listen_;

public:
    Wifi(WifiConnection &connection);

public:
    bool begin();
    void disable();

public:
    Listen &server() {
        return listen_;
    }

    bool disabled() {
        return !initialized_ || disabled_;
    }

public:
    inline static bool hasInternetAccess() {
        return WiFi.status() == WL_CONNECTED;
    }

    inline static bool discoveryEnabled() {
        return WiFi.status() == WL_CONNECTED || WiFi.status() == WL_AP_CONNECTED;
    }

};

}

#endif
