#ifndef FK_HTTP_POST_H_INCLUDED
#define FK_HTTP_POST_H_INCLUDED

#include <WiFi101.h>

#include "transmissions.h"

namespace fk {

constexpr uint32_t WifiHttpPostTimeout = 5000;

class HttpPost : public TransmissionTask {
private:
    uint32_t dieAt;
    WiFiClient wcl;

public:
    HttpPost();

public:
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
