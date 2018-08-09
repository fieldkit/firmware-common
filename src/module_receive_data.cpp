#include "module_receive_data.h"
#include "module_idle.h"

namespace fk {

void ModuleReceiveData::task() {
    lws::AlignedStorageBuffer<256> scratch;
    lws::VarintEncodedStream blockReader{ *services().reader, scratch.toBufferPtr() };

    services().pipe->clear();

    while (true) {
        services().alive();

        if (elapsed() > 1000) {
            transit<ModuleIdle>();
            return;
        }

        auto block = blockReader.read();
        if (block.eos()) {
            log("stream: End");
            transit<ModuleIdle>();
            return;
        }
        if (block) {
            log("stream: Read %ld bytes", block.blockSize);
            mark();
        }
    }
}

}
