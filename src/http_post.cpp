#include "http_post.h"
#include "utils.h"

namespace fk {

HttpPost::HttpPost(HttpTransmissionConfig &config) :
    TransmissionTask("HttpPost"), config(&config) {
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
        const auto length = strlen(config->url) + 1;
        char urlCopy[length];
        strncpy(urlCopy, config->url, length);
        Url parsed(urlCopy);

        parser.begin();

        // TODO: Verify we got good values? Though this should
        // probably have been checked before.
        if (parsed.server != nullptr && parsed.path != nullptr) {
            log("Connecting: '%s' / '%s'", parsed.server, parsed.path);

            if ((uint32_t)config->cachedAddress == (uint32_t)0) {
                if (!WiFi.hostByName(parsed.server, config->cachedAddress)) {
                    log("DNS failure on '%s'", parsed.server);
                }
            }

            // TODO: Fix blocking.
            if (config->cachedAddress != (uint32_t)0 && wcl.connect(config->cachedAddress, 80)) {
                PrintSizeCalculator sizeCalc;
                write(sizeCalc);
                log("Size: %d", sizeCalc.getSize());

                connected = true;
                log("Connected!");
                wcl.print("POST /");
                wcl.print(parsed.path);
                wcl.println(" HTTP/1.1");
                wcl.print("Host: ");
                wcl.println(parsed.server);
                wcl.print("Content-Type: ");
                wcl.println(getContentType());
                wcl.print("Content-Length: ");
                wcl.println(sizeCalc.getSize());
                wcl.println("Connection: close");
                wcl.println();
                write(wcl);
                wcl.flush();
            } else {
                log("Not connected!");
                return TaskEval::yield(retry);
            }
        } else {
            return TaskEval::error();
        }
    }

    while (wcl.available()) {
        auto c = wcl.read();
        parser.write(c);
    }

    if (!wcl.connected()) {
        wcl.stop();
        log("Done (statusCode=%d)", parser.getStatusCode());
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
