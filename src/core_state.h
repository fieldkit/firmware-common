#ifndef FK_CORE_STATE_H_INCLUDED
#define FK_CORE_STATE_H_INCLUDED

#include "app_messages.h"
#include "module_info.h"
#include "module_messages.h"
#include "two_wire_task.h"

namespace fk {

struct AvailableSensorReading {
    uint8_t sensor;
    SensorReading &reading;
};

class CoreState {
private:
    ModuleInfo modules[MaximumNumberOfModules];

public:
    CoreState();

public:
    ModuleInfo* attachedModules() {
        return modules;
    }
    size_t numberOfModules() const;
    size_t numberOfSensors() const;
    size_t numberOfReadings() const;

    void merge(uint8_t address, ModuleReplyMessage &reply);
    AvailableSensorReading getReading(size_t index);
    void clearReadings();

private:
    size_t getModuleIndex(uint8_t address);

};
}

#endif
