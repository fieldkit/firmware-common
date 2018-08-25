#include "wifi_startup.h"
#include "wifi_try_network.h"
#include "wifi_listening.h"
#include "transmit_files.h"
#include "wifi.h"

namespace fk {

void WifiStartup::task() {
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
