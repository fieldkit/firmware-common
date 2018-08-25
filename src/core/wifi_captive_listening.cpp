#include "wifi_captive_listening.h"
#include "wifi_listening.h"

namespace fk {

void WifiCaptiveListening::task() {
    if (fk_uptime() - began_ > WifiCaptivitiyTimeout) {
        transit_into<WifiListening>();
        return;
    }

    serve();
}

}
