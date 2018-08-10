#include "module_receive_data.h"
#include "module_idle.h"
#include "message_buffer.h"
#include "checksum_streams.h"
#include "tuning.h"

namespace fk {

void ModuleReceiveData::task() {
    auto reader = services().reader;
    Crc32Reader crc32{ *reader };

    auto received = 0;

    services().pipe->clear();

    while (true) {
        services().alive();

        if (elapsed() > TwoWireStreamingWait) {
            break;
        }

        uint8_t buffer[64];
        auto s = crc32.read(buffer, sizeof(buffer));
        if (s == lws::Stream::EOS) {
            log("stream: End");
            break;
        }
        if (s > 0) {
            received += s;
            mark();
        }
    }

    log("stream: Done (received %d) (checksum = 0x%lx)", received, crc32.checksum());

    services().incoming->clear();
    services().outgoing->clear();

    transit<ModuleIdle>();
}

}
