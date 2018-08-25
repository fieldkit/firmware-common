#include <WiFiUdp.h>

#include "discovery.h"
#include "device_id.h"
#include "wifi_tools.h"

namespace fk {

Discovery::Discovery() : Task("Discovery") {
}

TaskEval Discovery::task() {
    if (pingAt < fk_uptime()) {
        if (Wifi::discoveryEnabled()) {
            ping();
        }
        pingAt = fk_uptime() + DiscoveryPingInterval;
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
