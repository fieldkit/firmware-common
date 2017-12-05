#include "simple_ntp.h"
#include "debug.h"

namespace fk {

constexpr uint64_t SeventyYears = 2208988800UL;
constexpr uint32_t SimpleNTPPacketSize = 48;
constexpr uint32_t RetryAfter = 5000;

SimpleNTP::SimpleNTP(Clock &clock) : Task("NTP"), clock(&clock) {
}

void SimpleNTP::start() {
    if (!initialized) {
        udp.begin(2390);
        initialized = true;
    }

    send();
}

bool SimpleNTP::send() {
    uint8_t buffer[SimpleNTPPacketSize];

    fk_memzero(buffer, sizeof(buffer));

    buffer[0] = 0b11100011;   // LI, Version, Mode
    buffer[1] = 0;            // Stratum, or type of clock
    buffer[2] = 6;            // Polling Interval
    buffer[3] = 0xEC;         // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    buffer[12]  = 49;
    buffer[13]  = 0x4E;
    buffer[14]  = 49;
    buffer[15]  = 52;

    udp.beginPacket(address, 123);
    udp.write(buffer, sizeof(buffer));
    udp.endPacket();

    return true;
}

TaskEval SimpleNTP::task() {
    if (WiFi.status() == WL_AP_CONNECTED || WiFi.status() == WL_CONNECTED) {
        if (lastSent == 0 || millis() - lastSent > RetryAfter) {
            log("Asking for time...");
            start();
            lastSent = millis();
        }

        if (udp.parsePacket()) {
            uint8_t buffer[SimpleNTPPacketSize];

            udp.read(buffer, sizeof(buffer));

            // Pull time from the packet. Stored as a DWORD here as seconds since 1/1/1900
            auto high = word(buffer[40], buffer[41]);
            auto low = word(buffer[42], buffer[43]);
            auto secondsSince1900 = high << 16 | low;

            // Rezero to get UnixTime.
            auto epoch = secondsSince1900 - SeventyYears;
            clock->setTime(epoch);

            log("UTC: %d", epoch);

            return TaskEval::done();
        }
    }
    return TaskEval::yield();
}

}
