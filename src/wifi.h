#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "wifi_connections.h"
#include "app_servicer.h"

namespace fk {

class Wifi : public ActiveObject {
private:
    NetworkSettings *settings;
    Listen listen;

public:
    Wifi(NetworkSettings &settings, AppServicer &servicer);

public:
    void begin();

};

}

#endif
