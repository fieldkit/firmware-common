#include "discovery.h"

namespace fk {

Discovery::Discovery(Wifi &wifi) : Task("Discovery"), wifi(&wifi) {
}

void Discovery::enqueued() {
    pingAt = millis() + 5000;
}

TaskEval Discovery::task() {
    if (pingAt < millis()) {
        if (!wifi->isDisabled()) {
            ping();
        }
        pingAt = millis() + 5000;
    }

    return TaskEval::yield();
}

void Discovery::ping() {
    log("PING");

    // TODO: Fix hack to get the broadcast address.
    IPAddress ip = WiFi.localIP();
    auto destination = IPAddress(ip[0], ip[1], ip[2], 255);

    // log("Ping");

    // Why is this API like this? So weird.
    WiFiUDP udp;
    if (udp.begin(12345)) {
        udp.beginPacket(destination, FK_CORE_PORT_UDP);
        udp.write(".");
        udp.endPacket();
        udp.stop();
    }
    log("DONE");
}

}
