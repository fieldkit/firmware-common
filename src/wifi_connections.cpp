#include "wifi_connections.h"
#include "utils.h"

namespace fk {

ReadAppQuery::ReadAppQuery(WifiConnection &connection, AppServicer &servicer, TaskQueue &taskQueue) :
    Task("ReadAppQuery"), connection(&connection), servicer(&servicer), taskQueue(&taskQueue) {
}

TaskEval ReadAppQuery::task() {
    if (dieAt == 0) {
        dieAt = millis() + ConnectionTimeout;
    }
    else if (millis() > dieAt) {
        connection->close();
        log("Connection timed out.");
        return TaskEval::error();
    }

    auto bytesRead = connection->read();
    if (bytesRead > 0) {
        log("Read %d bytes", bytesRead);
        if (!servicer->handle(connection->getBuffer())) {
            connection->close();
            log("Error parsing query");
            return TaskEval::error();
        } else {
            return servicer->task();
        }
    }

    return TaskEval::idle();
}

HandleConnection::HandleConnection(AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue)
    : Task("HandleConnection"), servicer(&servicer), connection(&connection), readAppQuery(connection, servicer, taskQueue) {
}

void HandleConnection::enqueued() {
    readAppQuery.enqueued();
}

TaskEval HandleConnection::task() {
    return readAppQuery.task();
}

void HandleConnection::done() {
    connection->flush();

    if (connection->isOpen()) {
        log("Stop connection");
        connection->close();
    }
    else {
        log("No connection!");
    }
}

Listen::Listen(uint16_t port, AppServicer &servicer, WifiConnection &connection, TaskQueue &taskQueue)
    : Task("Listen"), server(port), servicer(&servicer), connection(&connection), taskQueue(&taskQueue), handleConnection(servicer, connection, taskQueue) {
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
    if (state == ListenerState::Busy) {
        if (connection->isOpen()) {
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
            taskQueue->push(handleConnection);
            return TaskEval::idle();
        }
    }

    return TaskEval::idle();
}

}
