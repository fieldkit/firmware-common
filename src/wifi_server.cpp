#include "wifi_server.h"
#include "utils.h"

namespace fk {

Listen::Listen(uint16_t port, AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue)
    : Task("Listen"), server(port), servicer(&servicer), connection(&connection), taskQueue(&taskQueue) {
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
    return millis() - lastActivity > WifiInactivityTimeout;
}

TaskEval Listen::task() {
    if (state == ListenerState::Busy) {
        if (connection->isConnected()) {
            return TaskEval::busy();
        }

        state = ListenerState::Disconnected;
    }

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
            connection->setConnection(wcl);
            taskQueue->append(*servicer);
            return TaskEval::idle();
        }
    }

    return TaskEval::idle();
}

}
