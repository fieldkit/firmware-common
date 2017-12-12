#include "discovery.h"

namespace fk {

Discovery::Discovery() : Task("Discovery") {
}

void Discovery::enqueued() {
    pingAt = millis() + 5000;
}

TaskEval Discovery::task() {
    if (pingAt < millis()) {
        ping();
        pingAt = millis() + 5000;
    }

    return TaskEval::yield();
}

void Discovery::ping() {
    // TODO: Fix hack to get the broadcast address.
    IPAddress ip = WiFi.localIP();
    IPAddress destination(ip[0], ip[1], ip[2], 255);

    // log("Ping");

    // Why is this API like this? So weird.
    WiFiUDP udp;
    if (udp.begin(12345)) {
        udp.beginPacket(destination, FK_CORE_PORT_UDP);
        udp.write(".");
        udp.endPacket();
        udp.stop();
    }
}

}
