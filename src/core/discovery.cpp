#include <WiFiUdp.h>

#include "discovery.h"
#include "device_id.h"
#include "wifi_tools.h"
#include "configuration.h"

namespace fk {

Discovery::Discovery() : Task("Discovery") {
}

TaskEval Discovery::task() {
    if (Wifi::discoveryEnabled()) {
        if (fk_uptime() > scheduled_) {
            ping();

            packets_++;

            if (packets_ == DiscoveryPackets) {
                packets_ = 0;
                scheduled_ = fk_uptime() + DiscoveryPingInterval;
            }
            else {
                scheduled_ = fk_uptime() + DiscoveryPacketInterval;
            }
        }
    }

    return TaskEval::idle();
}

void Discovery::ping() {
    auto destination = IPAddress(255, 255, 255, 255);

    // Why is this API like this? So weird.
    WiFiUDP udp;
    if (udp.begin(DiscoveryUdpPort)) {
        if (configuration.logging.discovery) {
            auto ipv4 = IpAddress4 { (uint32_t)destination };
            trace("Ping: %s", ipv4.toString());
        }

        udp.beginPacket(destination, DiscoveryUdpPort);

        if (false) {
            auto length = deviceId.length();
            uint8_t buffer[length + 1];
            memcpy(buffer, deviceId.toBuffer(), length);
            buffer[length + 0] = packets_;
            udp.write(buffer, sizeof(buffer));
        }
        else {
            udp.write(deviceId.toBuffer(), deviceId.length());
        }

        udp.endPacket();
        udp.stop();
    }
}

}
