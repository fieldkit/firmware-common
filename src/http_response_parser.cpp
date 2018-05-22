#include "http_response_parser.h"

namespace fk {

void HttpResponseParser::begin() {
    buffer[0] = pos = spacesSeen = 0;
    statusCode = 0;
}

void HttpResponseParser::write(uint8_t c) {
    if (spacesSeen < 2) {
        if (c == ' ') {
            spacesSeen++;
            if (spacesSeen == 2) {
                statusCode = atoi(buffer);
            }
            buffer[0] = pos = 0;
        } else {
            if (pos < MaxStatusCodeLength - 1) {
                buffer[pos++] = c;
                buffer[pos] = 0;
            }
        }
    }
}

}
