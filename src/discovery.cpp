#include "discovery.h"
#include "device_id.h"
#include "utils.h"

namespace fk {

Discovery::Discovery(Wifi &wifi) : Task("Discovery"), wifi(&wifi) {
}

TaskEval Discovery::task() {
    if (pingAt < millis()) {
        if (wifi->discoveryEnabled()) {
            ping();
        }
        pingAt = millis() + DiscoveryPingInterval;
    }

    return TaskEval::idle();
}

void Discovery::ping() {
    // IPAddress ip = WiFi.localIP();
    auto destination = IPAddress(255, 255, 255, 255);

    // Why is this API like this? So weird.
    WiFiUDP udp;
    if (udp.begin(DiscoveryUdpPort)) {
        if (false) {
            auto ipv4 = IpAddress4 { (uint32_t)destination };
            trace("Ping: %s", ipv4.toString());
        }
        udp.beginPacket(destination, DiscoveryUdpPort);
        udp.write(deviceId.toBuffer(), deviceId.length());
        udp.endPacket();
        udp.stop();
    }
}

}
