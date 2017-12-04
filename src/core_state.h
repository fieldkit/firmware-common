#ifndef FK_CORE_STATE_H_INCLUDED
#define FK_CORE_STATE_H_INCLUDED

#include "app_messages.h"
#include "module_info.h"
#include "module_messages.h"

namespace fk {

class CoreState {
private:
    ModuleInfo modules[MaximumNumberOfModules];

public:
    CoreState();
    void merge(uint8_t address, ModuleReplyMessage &reply);
    ModuleInfo* attachedModules() {
        return modules;
    }

    size_t numberOfModules() const {
        for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
            if (modules[i].address == 0) {
                return i;
            }
        }
        return MaximumNumberOfModules;
    }

    size_t numberOfSensors() const {
        size_t number = 0;
        for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
            if (modules[i].address > 0) {
                number += modules[i].numberOfSensors;
            }
        }
        return number;
    }

private:
    size_t getModuleIndex(uint8_t address);

};
}

#endif
