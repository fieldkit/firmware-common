#ifndef FK_PENDING_READINGS_H_INCLUDED
#define FK_PENDING_READINGS_H_INCLUDED

#include "module_info.h"

namespace fk {

class PendingReadings {
private:
    ModuleInfo *info_;
    size_t remaining_{ 0 };

public:
    PendingReadings(ModuleInfo &info);

public:
    void begin(size_t remaining);

    void done(size_t i, float value);

    size_t remaining() const;

};

}

#endif
