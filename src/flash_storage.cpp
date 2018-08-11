#include "debug.h"
#include "flash_storage.h"

namespace fk {

bool SerialFlashFileSystem::initialize(uint8_t cs, phylum::sector_index_t sector_size) {
    if (!storage_.initialize(cs, sector_size)) {
        FlashLog::error("Initialize failed");
        return false;
    }

    auto g = storage_.geometry();
    FlashLog::info("Flash Geometry: (%d x %lu)", g.number_of_blocks, g.block_size());

    if (!storage_.open()) {
        FlashLog::error("Open failed");
        return false;
    }

    if (!allocator_.initialize()) {
        FlashLog::error("Initialize failed");
        return false;
    }

    return true;
}

bool SerialFlashFileSystem::erase() {
    FlashLog::info("Erasing");
    if (!storage_.erase()) {
        return false;
    }

    return true;
}

bool SerialFlashFileSystem::reclaim(phylum::UnusedBlockReclaimer &reclaimer) {
    // reclaimer.walk(location);

    return true;
}

bool SerialFlashFileSystem::busy(uint32_t elapsed) {
    watchdog_->task();

    return true;
}

}
