#include "wifi.h"

namespace fk {

Wifi::Wifi(CoreState &state, AppServicer &servicer)
    : ActiveObject("Wifi", listen), state(&state), listen(ServerPort, servicer) {
}

void Wifi::begin() {
    WiFi.setPins(8, 7, 4, 2);

    if (WiFi.status() == WL_NO_SHIELD) {
        log("No wifi");
        return;
    } else {
        auto settings = state->getNetworkSettings();
        if (settings.createAccessPoint) {
            log("Creating AP");

            auto status = WiFi.beginAP(settings.ssid);
            if (status != WL_AP_LISTENING) {
                log("Error creating AP");
                return;
            }
        } else {
            log("Connecting to AP");
            WiFi.begin(settings.ssid, settings.password);
            log("Waiting on connection...");
        }
    }

    listen.begin();
}

}
