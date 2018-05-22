#include "http_response_writer.h"

namespace fk {

HttpResponseWriter::HttpResponseWriter(WiFiClient &wcl) : wcl(wcl) {
}

void HttpResponseWriter::writeHeaders(Url &url, OutgoingHttpHeaders &headers) {
    wcl.print("POST /");
    wcl.print(url.path);
    wcl.println(" HTTP/1.1");

    wcl.print("Host: ");
    wcl.println(url.server);

    wcl.println("Connection: close");

    wcl.print("Content-Length: ");
    wcl.println(headers.contentLength);

    if (headers.contentType != nullptr) {
        wcl.print("Content-Type: ");
        wcl.println(headers.contentType);
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
