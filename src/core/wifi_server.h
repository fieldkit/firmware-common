#ifndef FK_WIFI_SERVER_H_INCLUDED
#define FK_WIFI_SERVER_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>

#include "wifi_client.h"

namespace fk {

class Listen {
private:
    bool initialized_{ false };
    uint16_t port_{ 0 };
    WiFiServer server_;
    WifiConnection *connection_;

public:
    Listen(uint16_t port, WifiConnection &connection);

private:
    void begin();

public:
    bool listen();

    void end();
};

}

#endif
