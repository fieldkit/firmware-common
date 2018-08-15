#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"
#include "core_state.h"
#include "file_writer.h"

namespace fk {

class FirmwareStorage {
private:
    FlashState<PersistedState> *flashState_;
    phylum::AllocatedBlockedFile opened_;
    SerialFlashFileSystem *fs_;
    FileWriter writer_;

public:
    FirmwareStorage(FlashState<PersistedState> &flashState, SerialFlashFileSystem &fs);

public:
    lws::Writer *write();
    lws::Reader *read(FirmwareBank bank);

public:
    bool header(FirmwareBank bank, firmware_header_t &header);
    bool update(FirmwareBank bank, lws::Writer *writer, const char *etag);
    bool backup();

};

}

#endif
