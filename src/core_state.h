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

    size_t numberOfReadings() const {
        size_t number = 0;
        for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
            if (modules[i].address > 0) {
                for (size_t j = 0; j < modules[i].numberOfSensors; ++j) {
                    if (modules[i].readings[j].status == SensorReadingStatus::Done) {
                        number++;
                    }
                }
            }
        }
        return number;
    }

    AvailableSensorReading getReading(size_t index) {
        size_t number = 0;
        for (uint8_t i = 0; i < MaximumNumberOfModules; ++i) {
            if (modules[i].address > 0) {
                for (uint8_t j = 0; j < modules[i].numberOfSensors; ++j) {
                    if (modules[i].readings[j].status == SensorReadingStatus::Done) {
                        if (number == index) {
                            return AvailableSensorReading { j, modules[i].readings[j] };
                        }
                        number++;
                    }
                }
            }
        }

        fk_assert(false);

        return AvailableSensorReading { 0, modules[0].readings[0] };
    }

    void clearReadings() {
        for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
            for (size_t j = 0; j < MaximumNumberOfSensors; ++j) {
                modules[i].readings[j].status = SensorReadingStatus::Idle;
            }
        }
    }

private:
    size_t getModuleIndex(uint8_t address);

};
}

#endif
