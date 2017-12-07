#ifndef FK_HTTP_POST_H_INCLUDED
#define FK_HTTP_POST_H_INCLUDED

#include <WiFi101.h>

#include "transmissions.h"

namespace fk {

class DnsResolution {
private:
    const char *hostName;
    IPAddress ip;

public:
    DnsResolution(const char *hostName) : hostName(hostName), ip((uint32_t)0) {
    }

public:
    IPAddress &ipAddress() {
        return ip;
    }

    bool tryResolve() {
        if (ip != 0) {
            return true;
        }
        return WiFi.hostByName(hostName, ip);
    }
};

constexpr uint32_t WifiHttpPostTimeout = 5000;

class HttpPost : public TransmissionTask {
private:
    uint32_t dieAt{ 0 };
    Delay retry{ 500 };
    bool connected{ false };
    WiFiClient wcl;

public:
    HttpPost();

public:
    void done() override;
    void error() override;
    TaskEval task() override;

};

}

#endif
