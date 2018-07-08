#include "wifi_server.h"
#include "utils.h"

namespace fk {

Listen::Listen(uint16_t port, AppServicer &servicer, WifiConnection &connection)
    : Task("Listen"), port(port), server(port), servicer(&servicer), connection(&connection) {
}

void Listen::begin() {
    if (state == ListenerState::Idle) {
        lastActivity = millis();
        server.begin();
        log("Server began (port = %d)", port);
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

TaskEval Listen::task() {
    if (state == ListenerState::Busy) {
        if (connection->isConnected()) {
            return TaskEval::busy();
        }

        state = ListenerState::Disconnected;
    }

    begin();

    if (state == ListenerState::Disconnected) {
        auto wcl = server.available();
        if (wcl) {
            lastActivity = millis();
            log("Accepted!");
            connection->setConnection(wcl);
            state = ListenerState::Busy;
            return TaskEval::idle();
        }
    }

    return TaskEval::idle();
}

}
