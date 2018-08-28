#ifndef FK_MODULE_CALLBACKS_H_INCLUDED
#define FK_MODULE_CALLBACKS_H_INCLUDED

#include "module_messages.h"

namespace fk {

struct ModuleStates {
    Deferred configure;
    Deferred readings;
    Deferred message;

    ModuleStates() {
    }

    ModuleStates(Deferred configure, Deferred readings, Deferred message = { }) : configure(configure), readings(readings), message(message) {
    }
};

struct ModuleReadingStatus {
    uint32_t backoff{ 0 };

    ModuleReadingStatus() {
    }

    ModuleReadingStatus(uint32_t backoff) : backoff(backoff) {
    }
};

class ModuleCallbacks {
public:
    virtual ModuleStates states() {
        return { };
    }

};

}

#endif
