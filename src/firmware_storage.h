#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"
#include "core_state.h"

namespace fk {

class FirmwareStorage {
private:

public:
    FirmwareStorage();

public:
    lws::Writer *write(FirmwareBank bank);
    lws::Reader *read(FirmwareBank bank);

};

}

#endif
