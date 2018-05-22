#ifndef FK_HTTP_RESPONSE_WRITER_H_INCLUDED
#define FK_HTTP_RESPONSE_WRITER_H_INCLUDED

#include <WiFi101.h>

#include "url_parser.h"

namespace fk {

struct OutgoingHttpHeaders {
    const char *contentType;
    uint32_t contentLength;
    const char *version;
    const char *build;
    const char *deviceId;
};

class HttpResponseWriter {
private:
    WiFiClient &wcl;

public:
    explicit HttpResponseWriter(WiFiClient &wcl);

public:
    void writeHeaders(Url &url, OutgoingHttpHeaders &headers);

};

}

#endif
