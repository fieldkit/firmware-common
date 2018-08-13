#include "firmware_storage.h"

namespace fk {

constexpr const char LogName[] = "Firmware";

using Logger = SimpleLog<LogName>;

FirmwareStorage::FirmwareStorage(FlashState<PersistedState> &flashState, SerialFlashFileSystem &fs): flashState_(&flashState), fs_(&fs) {
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

bool FirmwareStorage::header(FirmwareBank bank, firmware_header_t &header) {
    header.version = FIRMWARE_VERSION_INVALID;

    auto addr = flashState_->state().firmwares.banks[(int32_t)bank];
    if (!addr.valid()) {
        Logger::info("Bank %d: address is invalid.", bank);
        return false;
    }

    auto file = fs_->files().open(addr, phylum::OpenMode::Read);
    if (!file.exists()) {
        Logger::info("Bank %d: file missing (%lu:%lu).", bank, addr.block, addr.position);
        return false;
    }

    file.seek(UINT64_MAX);

    file.seek(0);

    Logger::info("Bank %d: (%lu:%lu) %lu bytes", bank, addr.block, addr.position, (uint32_t)file.size());

    auto bytes = file.read((uint8_t *)&header, sizeof(firmware_header_t));
    if (bytes != sizeof(firmware_header_t)) {
        return false;
    }

    if (header.version != 1) {
        return false;
    }

    return true;
}

bool FirmwareStorage::update(FirmwareBank bank, lws::Writer *writer, const char *etag) {
    auto beg = opened_.beginning();
    auto head = opened_.head();

    Logger::info("Bank %d: Saving (size=%lu) (beg=%lu:%lu, head=%lu:%lu)", bank,
                 (uint32_t)opened_.size(), beg.block, beg.position, head.block, head.position);

    writer->close();

    opened_.close();

    auto previousAddr = flashState_->state().firmwares.banks[(int32_t)bank];
    flashState_->state().firmwares.banks[(int32_t)bank] = beg;

    if (!flashState_->save()) {
        Logger::error("Error saving block");
    }

    if (previousAddr.valid()) {
        auto previousFile = fs_->files().open(previousAddr, phylum::OpenMode::Write);
        if (previousFile.exists()) {
            previousFile.erase_all_blocks();
        }
    }

    return true;
}

}
