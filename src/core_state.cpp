#include <fk-module-protocol.h>

#include "core_state.h"
#include "debug.h"

namespace fk {

CoreState::CoreState() {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        modules[i].address = 0;
    }
}

void CoreState::merge(uint8_t address, ModuleReplyMessage &reply) {
    switch (reply.m().type) {
    case fk_module_ReplyType_REPLY_CAPABILITIES: {
        // TODO: Add REPLY_SENSOR_CAPABILITIES
        auto index = getModuleIndex(address);
        auto& module = modules[index];
        if (reply.m().capabilities.version > 0) {
            module.address = address;
            module.numberOfSensors = reply.m().capabilities.numberOfSensors;
            strncpy(module.name, (const char *)reply.m().capabilities.name.arg, sizeof(module.name));
        }
        else {
            auto sensorIndex = reply.m().sensorCapabilities.id;
            auto& sensor = module.sensors[sensorIndex];
            strncpy(sensor.name, (const char *)reply.m().sensorCapabilities.name.arg, sizeof(sensor.name));
            strncpy(sensor.unitOfMeasure, (const char *)reply.m().sensorCapabilities.unitOfMeasure.arg, sizeof(sensor.unitOfMeasure));
        }
        break;
    }
    default:
        break;
    }
}

size_t CoreState::getModuleIndex(uint8_t address) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address == 0 || modules[i].address == address) {
            return i;
        }
    }

    fk_assert(false);

    return 0;
}

}
