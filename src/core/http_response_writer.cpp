#include <cstdlib>

#include <Arduino.h>

#include "http_response_writer.h"

namespace fk {

HttpHeadersWriter::HttpHeadersWriter(Print *stream) : stream_(stream) {
}

void HttpHeadersWriter::writeHeaders(Url &url, const char *method, OutgoingHttpHeaders &headers) {
    stream_->print(method);
    stream_->print(" /");
    stream_->print(url.path);
    stream_->println(" HTTP/1.1");

    stream_->print("Host: ");
    stream_->println(url.server);

    stream_->println("Connection: close");

    if (headers.contentLength != OutgoingHttpHeaders::InvalidContentLength) {
        stream_->print("Content-Length: ");
        stream_->println(headers.contentLength);
    }

    if (headers.contentType != nullptr) {
        stream_->print("Content-Type: ");
        stream_->println(headers.contentType);
    }

    if (headers.etag != nullptr) {
        auto len = strlen(headers.etag);
        auto quote = headers.etag[0] != '"' && headers.etag[len - 1] != '"';
        stream_->print("If-None-Match: ");
        if (quote) {
            stream_->print('"');
        }
        stream_->print(headers.etag);
        if (quote) {
            stream_->print('"');
        }
        stream_->println();
    }

    if (headers.fileId != OutgoingHttpHeaders::InvalidFileId) {
        stream_->print("Fk-FileId: ");
        stream_->println(headers.fileId);
    }

    if (headers.version != nullptr) {
        stream_->print("Fk-Version: ");
        stream_->println(headers.version);
    }

    if (headers.build != nullptr) {
        stream_->print("Fk-Build: ");
        stream_->println(headers.build);
    }

    if (headers.deviceId != nullptr) {
        stream_->print("Fk-DeviceId: ");
        stream_->println(headers.deviceId);
    }

    if (headers.compiled > 0) {
        stream_->print("Fk-Compiled: ");
        stream_->println(headers.compiled);
    }

    stream_->println();
}

}
