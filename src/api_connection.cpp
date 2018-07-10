#include "api_connection.h"

namespace fk {

ApiConnection::ApiConnection(WifiConnection &connection, Pool &pool) : query_(&pool), reply_(&pool), connection_(&connection), pool_(&pool) {
}

void ApiConnection::entry() {
    bytesRead_ = 0;
    dieAt_ = 0;
}

void ApiConnection::task() {
    if (!service()) {
        // Go back unless we transitioned somewhere else.
        if (!transitioned()) {
            transit<WifiConnectionCompleted>();
        }
    }
}

bool ApiConnection::service() {
    if (!readQuery()) {
        flushAndClose();
        return false;
    }

    // NOTE: We depend on being able to read the query in one go.
    if (bytesRead_ == 0) {
        return true;
    }

    pool_->clear();
    reply_.clear();
    buffer().clear();

    if (handle())  {
        flushAndClose();
    }

    send_event(AppQueryEvent{ query_.m().type });

    return false;
}

bool ApiConnection::flushAndClose() {
    connection_->flush();

    if (connection_->isConnected()) {
        #if FK_LOGGING_VERBOSITY > 1
        log("Stop connection");
        #endif
        connection_->close();
    }
    else {
        log("No connection!");
    }

    return true;
}

bool ApiConnection::readQuery() {
    if (dieAt_ == 0) {
        dieAt_ = fk_uptime() + WifiConnectionTimeout;
    }
    else if (fk_uptime() > dieAt_) {
        connection_->close();
        log("Connection died.");
        return false;
    }

    auto read = connection_->read();
    if (read > 0) {
        #if FK_LOGGING_VERBOSITY > 1
        log("Read %d bytes", read);
        #endif
        bytesRead_ += read;
        MessageBuffer &buffer = connection_->getBuffer();
        if (!buffer.read(query_)) {
            connection_->close();
            log("Error parsing query");
            return false;
        }
    }

    return true;
}

bool ApiConnection::handle() {
    reply_.error("Busy");

    if (!buffer().write(reply_)) {
        log("Error writing reply");
    }

    return true;
}

}
