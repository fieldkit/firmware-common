#include "module_receive_data.h"
#include "module_idle.h"
#include "message_buffer.h"
#include "checksum_streams.h"
#include "firmware_storage.h"
#include "two_wire_child.h"
#include "tuning.h"

namespace fk {

constexpr uint32_t TwoWireStreamingTimeout = 3 * 1000;

ModuleReceiveData::ModuleReceiveData() {
}

ModuleReceiveData::ModuleReceiveData(ModuleCopySettings settings) : settings_(settings) {
}

void ModuleReceiveData::task() {
    FirmwareStorage firmwareStorage{ *services().flashState, *services().flashFs };

    auto fileWriter = firmwareStorage.write();
    auto writer = Crc32Writer{ *fileWriter };
    auto child = services().child;
    auto received = (uint32_t)0;
    auto &reader = child->reader();

    services().dataCopyStatus = DataCopyStatus{ };

    child->clear();

    mark();

    while (received < settings_.size) {
        services().alive();

        if (elapsed() > TwoWireStreamingTimeout) {
            log("Data stopped!");
            break;
        }

        uint8_t buffer[FileCopyBufferSize];
        auto s = reader.read(buffer, sizeof(buffer));
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

    fileWriter->close();

    if (received == settings_.size) {
        log("stream: Done (expected=%lu) (received=%lu) (bank=%d) (checksum=0x%lx)",
            settings_.size, received, settings_.bank, writer.checksum());
        services().dataCopyStatus.checksum = writer.checksum();
        services().dataCopyStatus.pending = firmwareStorage.beginningOfOpenFile();
    }
    else {
        log("stream: Fail (expected=%lu) (received=%lu)", settings_.size, received);
        firmwareStorage.erase(fileWriter);
    }

    log("Clearing (incoming=%d, outgoing=%d)", child->incoming().position(), child->outgoing().position());

    child->incoming().clear();

    child->outgoing().clear();

    transit<ModuleIdle>();
}

}
