#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <cstdint>

namespace fk {

constexpr uint8_t MaximumNumberOfModules = 8;
constexpr uint8_t MaximumNumberOfSensors = 32;
constexpr uint8_t MaximumModuleNameLength = 16;
constexpr uint8_t MaximumSensorNameLength = 16;
constexpr uint8_t MaximumSensorUoMLength = 8;

struct SensorInfo {
    char name[MaximumSensorNameLength];
    char unitOfMeasure[MaximumSensorUoMLength];
};

enum class SensorReadingStatus {
    Idle,
    Busy,
    Done,
};

struct SensorReading {
    uint32_t time;
    float value;
    SensorReadingStatus status;
};

struct ModuleInfo {
    uint8_t address;
    uint8_t numberOfSensors;
    char name[MaximumModuleNameLength];
    SensorInfo sensors[MaximumNumberOfSensors];
    SensorReading readings[MaximumNumberOfSensors];
};

}

#endif

