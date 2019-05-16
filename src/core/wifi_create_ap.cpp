#if defined(FK_CORE)
#include <Base64.h>
#endif
#include <WiFi101.h>
#include <IPAddress.h>

#include "wifi_tools.h"
#include "wifi_create_ap.h"
#include "wifi_disable.h"
#include "wifi_listening.h"

#include "device_id.h"

namespace fk {

static const char *Prefix = "FK-";

#if defined(FK_CORE)
static void getAccessPointName(char *name, size_t size) {
    auto length = base64_enc_len(deviceId.length());
    char unique[length + strlen(Prefix) + 1];
    strncpy(unique, Prefix, strlen(Prefix));
    base64_encode(unique + strlen(Prefix), (char *)deviceId.toBuffer(), deviceId.length());
    unique[length + strlen(Prefix) - 1] = 0;
    strncpy(name, unique, size);
}
#else
static void getAccessPointName(char *name, size_t size) {
    strncpy(name, "FK-UNKNOWN", size);
}
#endif

void WifiCreateAp::task() {
    char name[64];

    auto identity = services().state->getIdentity();
    if (strlen(identity.device) > 0) {
        strncpy(name, Prefix, strlen(Prefix));
        strncpy(name + strlen(Prefix), identity.device, sizeof(name) - strlen(Prefix) - 1);
    }
    else {
        getAccessPointName(name, sizeof(name));
    }

    log("Creating AP '%s'... (%s)", name, getWifiStatus());
    IPAddress ip{ 192, 168, 2, 1 };
    auto status = WiFi.beginAP(name, 2, ip);
    if (status != WL_AP_LISTENING) {
        transit<WifiDisable>();
        services().state->updateIp(0);
        return;
    }

    services().state->updateIp(WiFi.localIP());

    transit_into<WifiListening>();
}

}
