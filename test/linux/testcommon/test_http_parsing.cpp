#include <gtest/gtest.h>

#include "http_response_parser.h"

using namespace fk;

class HttpParsingSuite : public ::testing::Test {
protected:
    HttpResponseParser parser_;

};

const char *Header200Ok = R"END(HTTP/1.1 200 OK
Accept-Ranges: bytes
Cache-Control: public, max-age=0
Last-Modified: Fri, 27 Jul 2018 18:21:37 GMT
ETag: W/"5ec4-164dcf9852c"
Content-Type: application/octet-stream
Content-Length: 24260
Date: Fri, 27 Jul 2018 21:46:06 GMT
Connection: close


)END";

static inline void copy_lowercase(const char *src, size_t n, HttpResponseParser &parser) {
    for (auto i = 0; i < n; i++) {
        parser.write(tolower(*src));
        src++;
    }
}

static inline void copy(const char *src, size_t n, HttpResponseParser &parser) {
    for (auto i = 0; i < n; i++) {
        parser.write(*src);
        src++;
    }
}

TEST_F(HttpParsingSuite, Parsing) {
    parser_.begin();

    copy(Header200Ok, strlen(Header200Ok), parser_);

    ASSERT_FALSE(parser_.reading_header());
    ASSERT_EQ(parser_.status_code(), 200);
    ASSERT_EQ(parser_.content_length(), 24260);

    parser_.begin();

    copy(Header200Ok, strlen(Header200Ok) - 1, parser_);

    ASSERT_TRUE(parser_.reading_header());
    ASSERT_EQ(parser_.status_code(), 200);
    ASSERT_EQ(parser_.content_length(), 24260);

    parser_.begin();

    copy_lowercase(Header200Ok, strlen(Header200Ok), parser_);

    ASSERT_FALSE(parser_.reading_header());
    ASSERT_EQ(parser_.status_code(), 200);
    ASSERT_EQ(parser_.content_length(), 24260);
}
