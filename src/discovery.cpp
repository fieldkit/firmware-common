#include "discovery.h"
#include "device_id.h"

namespace fk {

constexpr uint32_t PingInterval = 2500;

Discovery::Discovery(TwoWireBus &bus, Wifi &wifi) : ActiveObject("Discovery"), bus(&bus), wifi(&wifi) {
}

void Discovery::idle() {
    if (pingAt < millis()) {
        if (!wifi->isDisabled()) {
            ping();
        }
        pingAt = millis() + PingInterval;
    }
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
