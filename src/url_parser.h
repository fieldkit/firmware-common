#ifndef FK_URL_PARSER_H_INCLUDED
#define FK_URL_PARSER_H_INCLUDED

#include <cstdint>
#include <cstdarg>

namespace fk {

struct Url {
private:
    static constexpr size_t BufferSize = 256;
    char buffer[BufferSize];

public:
    const char *server{ nullptr };
    const char *path{ nullptr };
    uint16_t port{ 80 };

public:
    Url(const char *f, ...) {
        va_list args;
        va_start(args, f);
        vsnprintf(buffer, sizeof(buffer), f, args);
        va_end(args);

        for (auto p = buffer; p[0] != 0 && p[1] != 0; ++p) {
            if (server == nullptr && p[0] == '/' && p[1] == '/') {
                p += 2;
                server = p;
            } else if (server != nullptr && p[0] == ':') {
                p[0] = 0;
                auto portBegin = ++p;
                for ( ; p[0] != 0; ++p) {
                    if (p[0] == '/') {
                        p[0] = 0;
                        port = atoi(portBegin);
                        path = p + 1;
                        break;
                    }
                }
                break;
            } else if (server != nullptr && p[0] == '/') {
                p[0] = 0;
                path = p + 1;
                break;
            }
        }
    }
};

}

#endif
