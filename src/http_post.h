#ifndef FK_HTTP_POST_H_INCLUDED
#define FK_HTTP_POST_H_INCLUDED

#include "wifi.h"
#include "transmissions.h"
#include "utils.h"

namespace fk {

constexpr uint32_t WifiHttpPostTimeout = 5000;

struct HttpTransmissionConfig {
    const char *url;
    const char *streamUrl;

    // This assume they're both the same! Maybe define this as hostname and then
    // two paths?
    CachedDnsResolution cachedDns;
};

class HttpPost : public TransmissionTask {
private:
    Wifi *wifi;
    HttpTransmissionConfig *config;
    HttpResponseParser parser;
    uint32_t dieAt{ 0 };
    Delay retry{ 500 };
    bool connected{ false };
    WiFiClient wcl;

public:
    HttpPost(Wifi &wifi, HttpTransmissionConfig &config);

public:
    void done() override;
    void error() override;
    TaskEval task() override;

};

}

#endif
