#include "wifi_startup.h"
#include "wifi_try_network.h"
#include "wifi_listening.h"
#include "transmit_files.h"
#include "wifi.h"

namespace fk {

WifiStartup::WifiStartup() {
    config_ = { };
}

WifiStartup::WifiStartup(WifiCheckConfig config) : config_(config) {
}

void WifiStartup::task() {
    services().config = config_;

    // Reset this because some transitions are unable to use transit_into.
    config_ = { };

    if (services().wifi->disabled()) {
        if (!services().wifi->begin()) {
            error("Wifi initialize failed");
        }

        transit_into<WifiTryNetwork>((uint8_t)0);
    }
    else {
        if (services().wifi->hasInternetAccess()) {
            transit_into<WifiTransmitFiles>();
        }
        else {
            transit_into<WifiListening>();
        }
    }
}

}
