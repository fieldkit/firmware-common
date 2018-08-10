#include "module_receive_data.h"
#include "module_idle.h"
#include "message_buffer.h"

namespace fk {

void ModuleReceiveData::task() {
    auto reader = services().reader;
    auto received = 0;

    services().pipe->clear();

    while (true) {
        services().alive();

        if (elapsed() > 500) {
            transit<ModuleIdle>();
            break;
        }

        uint8_t buffer[64];
        auto s = reader->read(buffer, sizeof(buffer));
        if (s == lws::Stream::EOS) {
            log("stream: End");
            transit<ModuleIdle>();
            break;
        }
        if (s > 0) {
            received += s;
            mark();
        }
    }

    log("stream: Done (received %d)", received);
    services().incoming->clear();
    services().outgoing->clear();
}

}
