#ifndef FK_HTTP_RESPONSE_WRITER_H_INCLUDED
#define FK_HTTP_RESPONSE_WRITER_H_INCLUDED

#include "url_parser.h"

class Print;

namespace fk {

struct OutgoingHttpHeaders {
    static constexpr uint32_t InvalidContentLength = (uint32_t)-1;
    static constexpr uint8_t InvalidFileId = (uint8_t)-1;

    const char *contentType;
    const char *version;
    const char *build;
    const char *deviceId;
    const char *etag;
    uint32_t compiled;
    uint32_t contentLength{ InvalidContentLength };
    uint8_t fileId{ InvalidFileId };

    OutgoingHttpHeaders(const char *contentType, uint32_t contentLength, const char *version,
                        const char *build, uint32_t compiled, const char *deviceId, uint8_t fileId) :
        contentType(contentType), version(version), build(build), deviceId(deviceId), etag(nullptr), compiled(compiled), contentLength(contentLength), fileId(fileId) {
    }

    OutgoingHttpHeaders(const char *contentType, const char *version, const char *build,
                        uint32_t compiled, const char *deviceId, const char *etag) :
        contentType(contentType), version(version), build(build), deviceId(deviceId), etag(etag), compiled(compiled) {
    }
};

class HttpHeadersWriter {
private:
    Print *stream_;

public:
    explicit HttpHeadersWriter(Print *stream);

public:
    void writeHeaders(Url &url, const char *method, OutgoingHttpHeaders &headers);

};

}

#endif
