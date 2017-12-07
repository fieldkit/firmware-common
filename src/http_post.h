#ifndef FK_HTTP_POST_H_INCLUDED
#define FK_HTTP_POST_H_INCLUDED

#include <WiFi101.h>

#include "transmissions.h"

namespace fk {

constexpr uint32_t WifiHttpPostTimeout = 5000;

struct HttpTransmissionConfig {
    const char *url;
    ::IPAddress cachedAddress;
};

class HttpPost : public TransmissionTask {
private:
    HttpTransmissionConfig *config;
    uint32_t dieAt{ 0 };
    Delay retry{ 500 };
    bool connected{ false };
    WiFiClient wcl;

public:
    HttpPost(HttpTransmissionConfig &config);

public:
    void done() override;
    void error() override;
    TaskEval task() override;

};

}

#endif
