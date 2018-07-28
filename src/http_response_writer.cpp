#include "http_response_writer.h"

namespace fk {

HttpHeadersWriter::HttpHeadersWriter(WiFiClient &wcl) : wcl(wcl) {
}

void HttpHeadersWriter::writeHeaders(Url &url, const char *method, OutgoingHttpHeaders &headers) {
    wcl.print(method);
    wcl.print(" /");
    wcl.print(url.path);
    wcl.println(" HTTP/1.1");

    wcl.print("Host: ");
    wcl.println(url.server);

    wcl.println("Connection: close");

    if (headers.contentLength != OutgoingHttpHeaders::InvalidContentLength) {
        wcl.print("Content-Length: ");
        wcl.println(headers.contentLength);
    }

    if (headers.contentType != nullptr) {
        wcl.print("Content-Type: ");
        wcl.println(headers.contentType);
    }

    if (headers.etag != nullptr) {
        auto len = strlen(headers.etag);
        auto quote = headers.etag[0] != '"' && headers.etag[len - 1] != '"';
        wcl.print("If-None-Match: ");
        if (quote) {
            wcl.print('"');
        }
        wcl.print(headers.etag);
        if (quote) {
            wcl.print('"');
        }
        wcl.println();
    }

    if (headers.fileId != OutgoingHttpHeaders::InvalidFileId) {
        wcl.print("Fk-FileId: ");
        wcl.println(headers.fileId);
    }

    if (headers.version != nullptr) {
        wcl.print("Fk-Version: ");
        wcl.println(headers.version);
    }

    if (headers.build != nullptr) {
        wcl.print("Fk-Build: ");
        wcl.println(headers.build);
    }

    if (headers.deviceId != nullptr) {
        wcl.print("Fk-DeviceId: ");
        wcl.println(headers.deviceId);
    }

    wcl.println();
}

}
