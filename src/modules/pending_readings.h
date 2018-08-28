#ifndef FK_PENDING_READINGS_H_INCLUDED
#define FK_PENDING_READINGS_H_INCLUDED

#include "module_info.h"

namespace fk {

class PendingReadings {
private:
    ModuleInfo *info_;

public:
    PendingReadings(ModuleInfo &info);

public:
    void done(size_t i, float value);

    size_t remaining() const;

};

}

#endif
