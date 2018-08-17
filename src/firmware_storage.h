#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"
#include "core_state.h"
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
    bool header(FirmwareBank bank, firmware_header_t &header);
    bool update(FirmwareBank bank, lws::Writer *writer, const char *etag);
    bool backup();

};

}

#endif
