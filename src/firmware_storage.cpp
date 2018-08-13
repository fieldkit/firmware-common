#include "firmware_storage.h"

namespace fk {

constexpr const char LogName[] = "Firmware";

using Logger = SimpleLog<LogName>;

FirmwareStorage::FirmwareStorage(SerialFlashFileSystem &fs): fs_(&fs) {
}

lws::Writer *FirmwareStorage::write() {
    opened_ = fs_->files().open({ }, phylum::OpenMode::Write);

    if (!opened_.format()) {
        // NOTE: Just return a /dev/null writer?
        fk_assert(false);
        return nullptr;
    }

    writer_ = FileWriter{ opened_ };

    return &writer_;
}

lws::Reader *FirmwareStorage::read(FirmwareBank bank) {
    opened_ = fs_->files().open({ }, phylum::OpenMode::Read);

    return nullptr;
}

bool FirmwareStorage::update(FirmwareBank bank, lws::Writer *writer, const char *etag) {
    auto beg = opened_.beginning();
    auto head = opened_.head();

    Logger::info("Saving %d (size=%lu) (beg=%lu:%lu, head=%lu:%lu)", bank, (uint32_t)opened_.size(),
                 beg.block, beg.position, head.block, head.position);

    writer->close();
    opened_.close();

    opened_.erase_all_blocks();

    // TODO: Update bank address.
    // TODO: Save super block
    // TODO: Erase old file.
    return false;
}

}
