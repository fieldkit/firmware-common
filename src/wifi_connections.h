#ifndef FK_WIFI_CONNECTIONS_H_INCLUDED
#define FK_WIFI_CONNECTIONS_H_INCLUDED

#include <WiFi101.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "app_servicer.h"
#include "core_state.h"

namespace fk {

class HandleConnection : public AppServicer {
private:
    uint32_t dieAt { 0 };
    WiFiClient wcl;

public:
    HandleConnection(WiFiClient wcl, ModuleController &modules, LiveData &liveData, CoreState &state, Pool &pool);

    TaskEval task() override;
};

class Listen : public Task {
    static constexpr char Name[] = "Listen";

private:
    bool connected{ false };
    Pool pool;
    WiFiServer *server;
    ModuleController *modules;
    LiveData *liveData;
    CoreState *state;
    HandleConnection handleConnection;

public:
    Listen(WiFiServer &server, ModuleController &modules, LiveData &liveData, CoreState &state);

    TaskEval task() override;
};

}

#endif
