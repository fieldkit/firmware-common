#include "wifi_connections.h"
#include "wifi_message_buffer.h"
#include "utils.h"

namespace fk {

constexpr uint32_t ConnectionTimeout = 5000;
constexpr uint32_t ConnectionMemory = 128;

HandleConnection::HandleConnection(WiFiClient wcl, LiveData &liveData, CoreState &state, Pool &pool)
    : AppServicer("HandleConnection", liveData, state, pool), wcl(wcl) {
}

TaskEval HandleConnection::task() {
    if (dieAt == 0) {
        dieAt = millis() + ConnectionTimeout;
    } else if (millis() > dieAt) {
        wcl.stop();
        log("Connection timed out.");
        return TaskEval::error();
    }
    if (wcl.available()) {
        WifiMessageBuffer incoming;
        auto bytesRead = incoming.read(wcl);
        if (bytesRead > 0) {
            log("Read %d bytes", bytesRead);
            if (!read(incoming)) {
                wcl.stop();
                log("Error parsing query");
                return TaskEval::error();
            } else {
                auto e = AppServicer::task();
                if (!e.isIdle()) {
                    auto &buffer = outgoingBuffer();
                    auto bytesWritten = wcl.write(buffer.ptr(), buffer.position());
                    log("Wrote %d bytes", bytesWritten);
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

constexpr char Listen::Name[];

Listen::Listen(WiFiServer &server, LiveData &liveData, CoreState &state)
    : Task(Name), pool("WifiService", ConnectionMemory), server(&server),
      liveData(&liveData), state(&state), handleConnection(WiFiClient(), liveData, state, pool) {
}

TaskEval Listen::task() {
    if (WiFi.status() == WL_AP_CONNECTED || WiFi.status() == WL_CONNECTED) {
        if (!connected) {
            IpAddress4 ip{ WiFi.localIP() };
            log("Connected ip: %s", ip.toString());
            connected = true;
        }
    } else {
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
            handleConnection = HandleConnection{ wcl, *liveData, *state, pool };
            return TaskEval::pass(handleConnection);
        }
    }

    return TaskEval::idle();
}

}
