#ifndef FK_SIMPLE_NTP_H_INCLUDED
#define FK_SIMPLE_NTP_H_INCLUDED

#include <WiFi101.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "rtc.h"

namespace fk {

class Wifi;

class SimpleNTP : public Task {
    static constexpr size_t NumberOfAddresses = 2;

private:
    ClockType *clock;
    Wifi *wifi;
    WiFiUDP udp;
    IPAddress addresses[NumberOfAddresses] {
        IPAddress{129, 6, 15, 28},
        IPAddress{164, 67, 62, 194}
    };
    uint8_t addressIndex{ 0 };
    bool initialized{ false };
    uint32_t lastSent{ 0 };
    uint32_t started{ 0 };

public:
    SimpleNTP(ClockType &clock, Wifi &wifi);
    ~SimpleNTP();

public:
    void enqueued() override;
    TaskEval task() override;

private:
    void start();
    void stop();
    bool send();

};

}

#endif
