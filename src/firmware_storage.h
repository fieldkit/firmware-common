#ifndef FK_FIRMWARE_STORAGE_H_INCLUDED
#define FK_FIRMWARE_STORAGE_H_INCLUDED

#include <lwstreams/lwstreams.h>

#include "firmware_header.h"

namespace fk {

class FirmwareStorage {
private:

public:
    FirmwareStorage();

public:
    lws::Writer *write();
    lws::Reader *read();

};

}

#endif
