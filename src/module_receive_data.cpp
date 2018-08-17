#include "module_receive_data.h"
#include "module_idle.h"
#include "message_buffer.h"
#include "checksum_streams.h"
#include "firmware_storage.h"
#include "tuning.h"

namespace fk {

ModuleReceiveData::ModuleReceiveData() {
}

ModuleReceiveData::ModuleReceiveData(ModuleCopySettings settings) : settings_(settings) {
}

void ModuleReceiveData::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    auto reader = services().reader;
    auto fileWriter = firmwareStorage.write();
    auto writer = Crc32Writer{ *fileWriter };
    auto received = (uint32_t)0;

    services().pipe->clear();

    mark();

    while (received < settings_.size) {
        services().alive();

        if (elapsed() > TwoWireStreamingWait) {
            log("Data stopped!");
            break;
        }

        uint8_t buffer[64];
        auto s = reader->read(buffer, sizeof(buffer));
        if (s == lws::Stream::EOS) {
            log("stream: End");
            break;
        }
        if (s > 0) {
            writer.write(buffer, s);
            received += s;
            mark();
        }
    }

    if (received != settings_.size) {
        log("stream: Fail (expected=%lu) (received=%lu)", settings_.size, received);
    }
    else {
        log("stream: Done (expected=%lu) (received=%lu) (bank=%d) (checksum=0x%lx)",
            settings_.size, received, settings_.bank, writer.checksum());
        firmwareStorage.update(settings_.bank, fileWriter);
    }

    services().incoming->clear();

    services().outgoing->clear();

    transit<ModuleIdle>();
}

}
