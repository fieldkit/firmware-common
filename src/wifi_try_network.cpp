#include "wifi_try_network.h"
#include "wifi_sync_time.h"
#include "wifi_create_ap.h"
#include "utils.h"

namespace fk {

void WifiTryNetwork::task() {
    if (index_ >= MaximumRememberedNetworks) {
        log("No more networks (%s)", getWifiStatus());
        transit<WifiCreateAp>();
        return;
    }

    auto settings = services().state->getNetworkSettings();
    auto network = settings.networks[index_];
    if (network.ssid[0] == 0) {
        log("N[%d] No network configured (%s)", index_, getWifiStatus());
        transit_into<WifiTryNetwork>((uint8_t)(index_ + 1));
        return;
    }

    log("N[%d] Connecting to AP '%s'... (%s)", index_, network.ssid, getWifiStatus());
    if (WiFi.begin(network.ssid, network.password) != WL_CONNECTED) {
        log("N[%d] Failed (%s)", index_, getWifiStatus());
        transit_into<WifiTryNetwork>((uint8_t)(index_ + 1));
        return;
    }

    auto localIp = WiFi.localIP();
    services().state->updateIp(localIp);

    IpAddress4 ip{ localIp };
    log("N[%d] Connected (%s) (%s)", index_, getWifiStatus(), ip.toString());
    transit<WifiSyncTime>();
}

}
