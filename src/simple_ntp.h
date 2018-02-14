#ifndef FK_SIMPLE_NTP_H_INCLUDED
#define FK_SIMPLE_NTP_H_INCLUDED

#include <WiFi101.h>
#include <WiFiUdp.h>

#include "active_object.h"
#include "rtc.h"

namespace fk {

class Wifi;

class SimpleNTP : public Task {
private:
    Clock *clock;
    Wifi *wifi;
    WiFiUDP udp;
    IPAddress address{129, 6, 15, 28};
    bool initialized{ false };
    uint32_t lastSent{ 0 };

public:
    SimpleNTP(Clock &clock, Wifi &wifi);
    ~SimpleNTP();

public:
    TaskEval task() override;

public:
    void start();
    void stop();
    bool send();

};

}

#endif
