#ifndef FK_WIFI_H_INCLUDED
#define FK_WIFI_H_INCLUDED

#include "active_object.h"
#include "app_servicer.h"
#include "module_controller.h"

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

namespace fk {

class HandleConnection : public AppServicer {
    WiFiClient wcl;

public:
    HandleConnection(WiFiClient wcl, ModuleController &modules, Pool &pool);

    TaskEval task() override;
};

class Listen : public Task {
    static constexpr char Name[] = "Listen";

private:
    bool connected{ false };
    Pool pool;
    WiFiServer *server;
    ModuleController *modules;
    HandleConnection handleConnection;

public:
    Listen(WiFiServer &server, ModuleController &modules);

    TaskEval task() override;
};

struct NetworkSettings {
    const char *ssid;
    const char *password;
    uint16_t port;
};

class Wifi : public ActiveObject {
private:
    NetworkSettings *settings;
    ModuleController *modules;
    WiFiServer server;
    Listen listen;

public:
    Wifi(NetworkSettings &settings, ModuleController &modules);

    void begin();

};

}

#endif
