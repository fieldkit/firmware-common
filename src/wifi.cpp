#include "wifi.h"

namespace fk {

Wifi::Wifi(NetworkSettings &settings, ModuleController &modules, LiveData &liveData, CoreState &state)
    : ActiveObject("Wifi", listen), settings(&settings), modules(&modules), server(settings.port),
      listen(server, modules, liveData, state) {
}

void Wifi::begin() {
    WiFi.setPins(8, 7, 4, 2);

    if (WiFi.status() == WL_NO_SHIELD) {
        log("No wifi");
        return;
    } else {
        if (false) {
            log("Creating AP");

            auto status = WiFi.beginAP(settings->ssid);
            if (status != WL_AP_LISTENING) {
                log("Error creating AP");
                return;
            }
        } else {
            log("Connecting to AP");
            WiFi.begin(settings->ssid, settings->password);
            log("Waiting on connection...");
        }
    }

    server.begin();
}

}
