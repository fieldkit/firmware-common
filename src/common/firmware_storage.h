#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"
#include "flash_storage.h"
#include "file_reader.h"
#include "file_writer.h"

namespace fk {

class FirmwareStorage {
private:
    FlashStateService *flashState_;
    phylum::AllocatedBlockedFile opened_;
    SerialFlashFileSystem *fs_;
    FileWriter writer_;
    FileReader reader_;

public:
    FirmwareStorage(FlashStateService &flashState, SerialFlashFileSystem &fs);

private:
    MinimumFlashState& state() {
        return flashState_->minimum();
    }

public:
    lws::Writer *write();
    lws::SizedReader *read(FirmwareBank bank);

public:
    bool verify(phylum::BlockAddress address, uint32_t size);
    bool header(FirmwareBank bank, firmware_header_t &header);
    bool update(FirmwareBank bank, lws::Writer *writer);
    bool update(FirmwareBank bank, phylum::BlockAddress beginning);
    bool erase(lws::Writer *writer);
    bool backup();
    phylum::BlockAddress beginningOfOpenFile() {
        return opened_.beginning();
    }

};

}

#endif
