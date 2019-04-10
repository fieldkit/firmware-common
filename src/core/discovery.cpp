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
    /*
    static bool pinged{ false };

    if (Wifi::discoveryEnabled()) {
        if (!pinged) {
            if (Wifi::discoveryEnabled()) {
                auto pr = WiFi.ping("192.168.2.100");
                if (pr >= 0) {
                    log("Pinged! RTT = %d", pr);
                    pinged = true;
                } else {
                    log("Ping failed! Error code: %d", pr);
                }
            }
        }
    }
    */

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

        if (!udp.beginPacket(destination, DiscoveryUdpPort)) {
            error("Failed: beginPacket");
        }
        else {
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

            if (!udp.endPacket()) {
                error("Failed: endPacket");
            }
        }
        udp.stop();
    }
    else {
        error("Failed to start UDP");
    }
}

}
