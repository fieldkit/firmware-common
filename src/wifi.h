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

class Wifi : public ActiveObject {
public:
    static constexpr uint16_t ServerPort = 54321;

private:
    CoreState *state;
    Listen listen;

public:
    Wifi(CoreState &state, AppServicer &servicer);

public:
    void begin();

};

}

#endif
