#include "wifi_connection_completed.h"
#include "wifi_captive_listening.h"

namespace fk {

void WifiConnectionCompleted::task() {
    transit<WifiCaptiveListening>();
}

}
