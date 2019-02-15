#include "simple_ntp.h"
#include "debug.h"
#include "wifi.h"

namespace fk {

constexpr uint32_t NtpSyncInterval = 24 * Hours;
constexpr uint32_t NtpRetryAfter = 2 * Seconds;
constexpr uint64_t SeventyYears = 2208988800UL;
constexpr uint32_t PacketSize = 48;

SimpleNTP::SimpleNTP(ClockType &clock) : Task("NTP"), clock(&clock) {
}

SimpleNTP::~SimpleNTP() {
    stop();
}

void SimpleNTP::enqueued() {
    lastSent = 0;
    initialized = false;
}

TaskEval SimpleNTP::task() {
    if (!Wifi::hasInternetAccess()) {
            return TaskEval::done();
    }

    if (lastSynced > 0 && fk_uptime() - lastSynced < NtpSyncInterval) {
        return TaskEval::done();
    }

    if (lastSent == 0 || fk_uptime() - lastSent > NtpRetryAfter) {
        log("Asking for time...");
        start();
        lastSent = fk_uptime();
    }

    if (udp.parsePacket()) {
        uint8_t buffer[PacketSize];

        udp.read(buffer, sizeof(buffer));

        // Pull time from the packet. Stored as a DWORD here as seconds since 1/1/1900
        auto high = word(buffer[40], buffer[41]);
        auto low = word(buffer[42], buffer[43]);
        auto secondsSince1900 = high << 16 | low;

        // Rezero to get UnixTime.
        auto oldEpoch = clock->getTime();
        auto newEpoch = (uint32_t)(secondsSince1900 - SeventyYears);
        clock->setTime(newEpoch);

        FormattedTime newFormatted{ newEpoch };
        FormattedTime oldFormatted{ oldEpoch };
        log("UTC: '%s' -> '%s' (%" PRId64 "s)", oldFormatted.toString(), newFormatted.toString(), (int64_t)newEpoch - oldEpoch);

        lastSynced = fk_uptime();

        stop();

        return TaskEval::done();
    }

    return TaskEval::busy();
}

void SimpleNTP::start() {
    if (!initialized) {
        udp.begin(2390);
        initialized = true;
    }

    send();
}

void SimpleNTP::stop() {
    if (initialized) {
        udp.stop();
        initialized = false;
    }
}

bool SimpleNTP::send() {
    uint8_t buffer[PacketSize];

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

    udp.beginPacket(addresses[addressIndex % NumberOfAddresses], 123);
    udp.write(buffer, sizeof(buffer));
    udp.endPacket();

    addressIndex++;

    return true;
}

}
