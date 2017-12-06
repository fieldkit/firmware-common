#include "http_post.h"

namespace fk {

HttpPost::HttpPost() : TransmissionTask("HttpPost") {
}

void HttpPost::enqueued() {
    dieAt = millis() + WifiHttpPostTimeout;
    // TODO: Fix blocking.
    if (wcl.connect("code.conservify.org", 80)) {
        log("Connected!");
        wcl.println("GET /time/ HTTP/1.1");
        wcl.println("Host: code.conservify.org");
        wcl.println("Connection: close");
        wcl.println();
    } else {
        log("Not connected!");
    }
}

TaskEval HttpPost::task() {
    if (millis() > dieAt) {
        return TaskEval::error();
    }

    if (WiFi.status() == WL_AP_CONNECTED || WiFi.status() == WL_CONNECTED) {
        while (wcl.available()) {
            auto c = wcl.read();
            Serial.write(c);
        }

        if (!wcl.connected()) {
            return TaskEval::done();
        }
    }

    return TaskEval::yield();
}

}
