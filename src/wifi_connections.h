#ifndef FK_WIFI_CONNECTIONS_H_INCLUDED
#define FK_WIFI_CONNECTIONS_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"

namespace fk {

class HandleConnection : public Task {
private:
    uint32_t dieAt{ 0 };
    WiFiClient wcl;
    AppServicer *servicer;

public:
    HandleConnection(WiFiClient wcl, AppServicer &servicer);

public:
    TaskEval task() override;
};

class Listen : public Task {
    static constexpr char Name[] = "Listen";

private:
    bool connected{ false };
    Pool pool;
    WiFiServer *server;
    AppServicer *servicer;
    HandleConnection handleConnection;

public:
    Listen(WiFiServer &server, AppServicer &servicer);

public:
    TaskEval task() override;
};

}

#endif
