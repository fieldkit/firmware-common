#include "wifi_connections.h"
#include "utils.h"

namespace fk {

ReadAppQuery::ReadAppQuery(WiFiClient &wcl, AppServicer &servicer, WifiMessageBuffer &buffer) :
    Task("ReadAppQuery"), wcl(&wcl), servicer(&servicer), buffer(&buffer) {
}

TaskEval ReadAppQuery::task() {
    if (dieAt == 0) {
        dieAt = millis() + ConnectionTimeout;
    } else if (millis() > dieAt) {
        wcl->stop();
        log("Connection timed out.");
        return TaskEval::error();
    }
    else if (wcl->available()) {
        auto bytesRead = buffer->read();
        if (bytesRead > 0) {
            log("Read %d bytes", bytesRead);
            if (!servicer->handle(*buffer)) {
                wcl->stop();
                log("Error parsing query");
                return TaskEval::error();
            } else {
                return TaskEval::pass(*servicer);
            }
        }
    }

    return TaskEval::idle();
}

HandleConnection::HandleConnection(AppServicer &servicer)
    : ActiveObject("HandleConnection"), servicer(&servicer), buffer(), readAppQuery(wcl, servicer, buffer) {
}

void HandleConnection::enqueued() {
    push(readAppQuery);
}

void HandleConnection::done() {
    if (!buffer.empty()) {
        buffer.write();
    }

    if (wcl && wcl.connected()) {
        log("Stop connection");
        wcl.flush();
        wcl.stop();
    }
    else {
        log("No connection!");
    }
}

Listen::Listen(uint16_t port, AppServicer &servicer)
    : Task("Listen"), server(port),
      servicer(&servicer), handleConnection(servicer) {
}

void Listen::begin() {
    if (state == ListenerState::Idle) {
        lastActivity = millis();
        server.begin();
        log("Server began (possibly failed, though)");
        state = ListenerState::Disconnected;
    }
    else if (state != ListenerState::Disconnected) {
        log("Disconnected");
        state = ListenerState::Disconnected;
    }
}

void Listen::end() {
    if (state != ListenerState::Idle) {
        log("Ended");
        state = ListenerState::Idle;
    }
}

bool Listen::inactive() {
    if (state == ListenerState::Busy) {
        return false;
    }
    return millis() - lastActivity > InactivityTimeout;
}

TaskEval Listen::task() {
    begin();

    if (state == ListenerState::Disconnected) {
        // WiFiClient is 1480 bytes. Only has one buffer of the size
        // SOCKET_BUFFER_TCP_SIZE. Where SOCKET_BUFFER_TCP_SIZE is 1446.
        auto wcl = server.available();
        if (wcl) {
            lastActivity = millis();
            log("Accepted!");
            pool.clear();
            state = ListenerState::Busy;
            handleConnection.setConnection(wcl);
            return TaskEval::pass(handleConnection);
        }
    }

    return TaskEval::idle();
}

}
