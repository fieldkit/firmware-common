#include "http_post.h"

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

struct Url {
public:
    char *server{ nullptr };
    char *path{ nullptr };

public:
    Url(char *url) {
        for (auto p = url; p[0] != 0 && p[1] != 0; ++p) {
            if (server == nullptr && p[0] == '/' && p[1] == '/') {
                p += 2;
                server = p;
            } else if (server != nullptr && p[0] == '/') {
                p[0] = 0;
                path = p + 1;
                break;
            }
        }
    }
};

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
                connected = true;
                log("Connected!");
                wcl.print("GET /");
                wcl.print(parsed.path);
                wcl.println(" HTTP/1.1");
                wcl.print("Host: ");
                wcl.println(parsed.server);
                wcl.println("Connection: close");
                wcl.println();
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
        Serial.write(c);
    }

    if (!wcl.connected()) {
        wcl.stop();
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
