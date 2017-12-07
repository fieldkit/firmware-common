#include "http_post.h"

namespace fk {

HttpPost::HttpPost() : TransmissionTask("HttpPost") {
    done();
}

void HttpPost::error() {
    dieAt = 0;
    connected = false;
}

void HttpPost::done() {
    dieAt = 0;
    connected = false;
}

TaskEval HttpPost::task() {
    if (WiFi.status() != WL_AP_CONNECTED && WiFi.status() != WL_CONNECTED) {
        log("No Wifi, failing");
        return TaskEval::error();
    }

    if (dieAt == 0) {
        dieAt = millis() + WifiHttpPostTimeout;
    } else if (millis() > dieAt) {
        wcl.stop();
        return TaskEval::error();
    }

    if (!connected) {
        // TODO: Fix blocking.
        if (wcl.connect("code.conservify.org", 80)) {
            connected = true;
            log("Connected!");
            wcl.println("GET /time/ HTTP/1.1");
            wcl.println("Host: code.conservify.org");
            wcl.println("Connection: close");
            wcl.println();
        } else {
            log("Not connected!");
            return TaskEval::yield(retry);
        }
    }

    while (wcl.available()) {
        auto c = wcl.read();
        Serial.write(c);
    }

    if (!wcl.connected()) {
        wcl.stop();
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
