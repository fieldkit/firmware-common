#include "wifi.h"

namespace fk {

constexpr char Listen::Name[];

class WifiMessageBuffer : public MessageBuffer {
public:
    size_t read(WiFiClient &wcl) {
        auto pos = (size_t)wcl.read(ptr(), size());
        move(pos);
        return pos;
    }
};

class IpAddress4 {
private:
    char str[3 * 4 + 4] = { 0 };
    union {
        uint8_t bytes[4];
        uint32_t dword;
    } a;

public:
    IpAddress4() {
    }

    IpAddress4(uint32_t ip)  {
        a.dword = ip;
    }

public:
    const char *toString() {
        snprintf(str, sizeof(str), "%d.%d.%d.%d", a.bytes[0], a.bytes[1], a.bytes[2], a.bytes[3]);
        return str;
    }
};

HandleConnection::HandleConnection(WiFiClient wcl, ModuleController &modules, Pool &pool) : AppServicer(modules, pool), wcl(wcl) {
}

TaskEval HandleConnection::task() {
    if (wcl.available()) {
        WifiMessageBuffer incoming;
        auto bytesRead = incoming.read(wcl);
        if (bytesRead > 0) {
            debugfpln("Wifi", "Read %d bytes", bytesRead);
            if (!read(incoming)) {
                wcl.stop();
                debugfpln("Wifi", "Error parsing query");
                return TaskEval::error();
            }
            else {
                auto e = AppServicer::task();
                if (!e.isIdle()) {
                    auto& buffer = outgoingBuffer();
                    auto bytesWritten = wcl.write(buffer.ptr(), buffer.position());
                    debugfpln("Wifi", "Wrote %d bytes", bytesWritten);
                    fk_assert(bytesWritten == buffer.position());
                    buffer.clear();
                    wcl.stop();
                    return e;
                }
            }
        }
    }

    return TaskEval::idle();
}

Listen::Listen(WiFiServer &server, ModuleController &modules) :
    Task(Name), pool("WifiService", 128),
    server(&server), modules(&modules), handleConnection(WiFiClient(), modules, pool) {
}

TaskEval Listen::task() {
    if (WiFi.status() == WL_AP_CONNECTED || WiFi.status() == WL_CONNECTED) {
        if (!connected) {
            IpAddress4 ip { WiFi.localIP() };
            log("Connected ip: %s", ip.toString());
            connected = true;
        }
    }
    else {
        if (connected) {
            log("Disconnected");
            connected = false;
        }
    }

    if (connected) {
        // WiFiClient is 1480 bytes. Only has one buffer of the size
        // SOCKET_BUFFER_TCP_SIZE. Where SOCKET_BUFFER_TCP_SIZE is 1446.
        auto wcl = server->available();
        if (wcl) {
            log("Accepted!");
            pool.clear();
            handleConnection = HandleConnection { wcl, *modules, pool };
            return TaskEval::pass(handleConnection);
        }
    }

    return TaskEval::idle();
}

Wifi::Wifi(NetworkSettings &settings, ModuleController &modules)
    : ActiveObject("Wifi", listen), settings(&settings), modules(&modules), server(settings.port),
      listen(server, modules) {
}

void Wifi::begin() {
    WiFi.setPins(8, 7, 4, 2);

    if (WiFi.status() == WL_NO_SHIELD) {
        log("No wifi");
        return;
    }
    else {
        if (false) {
            log("Creating AP");

            auto status = WiFi.beginAP(settings->ssid);
            if (status != WL_AP_LISTENING) {
                log("Error creating AP");
                return;
            }
        }
        else {
            log("Connecting to AP");
            WiFi.begin(settings->ssid, settings->password);
            log("Waiting on connection...");
        }
    }

    server.begin();
}

}
