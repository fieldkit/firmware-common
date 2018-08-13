#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"
#include "core_state.h"
#include "file_writer.h"

namespace fk {

class FirmwareStorage {
private:
    phylum::AllocatedBlockedFile opened_;
    SerialFlashFileSystem *fs_;
    FileWriter writer_;

public:
    FirmwareStorage(SerialFlashFileSystem &fs);

public:
    lws::Writer *write();
    lws::Reader *read(FirmwareBank bank);
    bool update(FirmwareBank bank, lws::Writer *writer, const char *etag);

};

}

#endif
