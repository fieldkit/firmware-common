#include "wifi_connections.h"
#include "wifi_message_buffer.h"
#include "utils.h"

namespace fk {

constexpr uint32_t ConnectionTimeout = 5000;
constexpr uint32_t ConnectionMemory = 128;

HandleConnection::HandleConnection(WiFiClient wcl, AppServicer &servicer)
    : Task("HandleConnection"), wcl(wcl), servicer(&servicer) {
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
        WifiMessageBuffer buffer;
        auto bytesRead = buffer.read(wcl);
        if (bytesRead > 0) {
            log("Read %d bytes", bytesRead);
            if (!servicer->handle(buffer)) {
                wcl.stop();
                log("Error parsing query");
                return TaskEval::error();
            } else {
                auto e = servicer->task();
                if (!e.isIdle()) {
                    auto bytesWritten = wcl.write(buffer.ptr(), buffer.position());
                    log("Wrote %d bytes (%d)", bytesWritten, fk_free_memory());
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

Listen::Listen(uint16_t port, AppServicer &servicer)
    : Task(Name), pool("WifiService", ConnectionMemory), server(port),
      servicer(&servicer), handleConnection(WiFiClient(), servicer) {
}

void Listen::begin() {
    if (state == ListenerState::Idle) {
        server.begin();
        state = ListenerState::Disconnected;
        log("Server began (possibly failed, though)");
    } else {
        state = ListenerState::Disconnected;
    }
}

void Listen::end() {
    if (state != ListenerState::Idle) {
        log("Ended");
        state = ListenerState::Idle;
    }
}

TaskEval Listen::task() {
    begin();

    if (state == ListenerState::Disconnected) {
        // WiFiClient is 1480 bytes. Only has one buffer of the size
        // SOCKET_BUFFER_TCP_SIZE. Where SOCKET_BUFFER_TCP_SIZE is 1446.
        auto wcl = server.available();
        if (wcl) {
            log("Accepted!");
            pool.clear();
            state = ListenerState::Busy;
            handleConnection = HandleConnection{ wcl, *servicer };
            return TaskEval::pass(handleConnection);
        }
    }

    return TaskEval::idle();
}

}
