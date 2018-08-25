#ifndef FK_MODULE_COPY_SETTINGS_H_INCLUDED
#define FK_MODULE_COPY_SETTINGS_H_INCLUDED

#include "module_info.h"

namespace fk {

struct ModuleCopySettings {
    FirmwareBank bank;
    uint32_t size;
    const char *etag;
};

}

#endif
