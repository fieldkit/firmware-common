#include "wifi_server.h"
#include "utils.h"

namespace fk {

Listen::Listen(uint16_t port, WifiConnection &connection) : port_(port), server_(port), connection_(&connection) {
}

void Listen::begin() {
    if (!initialized_) {
        server_.begin();
        loginfof("Listen", "Server began (port = %d)", port_);
        initialized_ = true;
    }
}

void Listen::end() {
    if (initialized_) {
        initialized_ = false;
        loginfof("Listen", "Ended");
    }
}

bool Listen::listen() {
    begin();

    auto wcl = server_.available();
    if (wcl) {
        loginfof("Listen", "Accepted!");
        connection_->setConnection(wcl);
        return true;
    }

    return false;
}

}
