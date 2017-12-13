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

class StreamBuffer : public Print {
public:
    char *buffer;
    size_t size;
    size_t pos{ 0 };

public:
    StreamBuffer(char *buffer, size_t size) : buffer(buffer), size(size) {
    }

    size_t write(uint8_t c) override {
        if (pos < size) {
            buffer[pos++] = c;
            buffer[pos] = 0;
            return 1;
        }
        return 0;
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
                char buffer[1024];
                StreamBuffer stream(buffer, sizeof(buffer));
                write(stream);
                write(Serial);
                Serial.println("");

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
                wcl.println(stream.pos);
                wcl.println("Connection: close");
                wcl.println();
                wcl.print(stream.buffer);
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
        Serial.write(c);
    }

    if (!wcl.connected()) {
        wcl.stop();
        return TaskEval::done();
    }

    return TaskEval::idle();
}

}
