#ifdef FK_CORE
#include <Base64.h>
#endif
#include <WiFi101.h>
#include <IPAddress.h>

#include "wifi_tools.h"
#include "wifi_create_ap.h"
#include "wifi_disable.h"
#include "wifi_listening.h"

namespace fk {

#ifdef FK_CORE
static void getAccessPointName(char *name, size_t size) {
    auto length = base64_enc_len(deviceId.length());
    char unique[length + 3];
    unique[0] = 'F';
    unique[1] = 'K';
    unique[2] = '-';
    base64_encode(unique + 3, (char *)deviceId.toBuffer(), deviceId.length());
    unique[length + 3 - 1] = 0; // Trim '='
    strncpy(name, unique, size);
}
#else
static void getAccessPointName(char *name, size_t size) {
    strncpy(name, "FK-UNKONWN", size);
}
#endif

void WifiCreateAp::task() {
    // TODO: If done this before, skip?
    char name[32];
    getAccessPointName(name, sizeof(name));

    log("Creating AP '%s'... (%s)", name, getWifiStatus());
    IPAddress ip{ 192, 168, 2, 1 };
    auto status = WiFi.beginAP(name, 1, ip);
    if (status != WL_AP_LISTENING) {
        transit<WifiDisable>();
        services().state->updateIp(0);
        return;
    }

    services().state->updateIp(WiFi.localIP());

    transit_into<WifiListening>();
}

}
