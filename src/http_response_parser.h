#ifndef FK_HTTP_RESPONSE_PARSER_H_INCLUDED
#define FK_HTTP_RESPONSE_PARSER_H_INCLUDED

#include <WiFi101.h>

namespace fk {

class HttpResponseParser {
    static constexpr size_t MaxStatusCodeLength = 4;

private:
    // This only needs to be big enough to hold a status code.
    char buffer[MaxStatusCodeLength];
    uint8_t spacesSeen{ 0 };
    uint8_t pos{ 0 };
    uint16_t statusCode{ 0 };

public:
    void begin();
    void write(uint8_t c);

public:
    uint16_t getStatusCode() {
        return statusCode;
    }

};

}

#endif
