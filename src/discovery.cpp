#include "discovery.h"
#include "device_id.h"

namespace fk {

Discovery::Discovery(TwoWireBus &bus, Wifi &wifi) : Task("Discovery"), bus(&bus), wifi(&wifi) {
}

TaskEval Discovery::task() {
    if (pingAt < millis()) {
        if (!wifi->isDisabled()) {
            ping();
        }
        pingAt = millis() + PingInterval;
    }

    return TaskEval::idle();
}

void Discovery::ping() {
    // TODO: Fix hack to get the broadcast address.
    IPAddress ip = WiFi.localIP();
    auto destination = IPAddress(ip[0], ip[1], ip[2], 255);

    // Why is this API like this? So weird.
    WiFiUDP udp;
    if (udp.begin(FK_CORE_PORT_UDP)) {
        udp.beginPacket(destination, FK_CORE_PORT_UDP);
        udp.write(deviceId.toBuffer(), deviceId.length());
        udp.endPacket();
        udp.stop();
    }
}

}
