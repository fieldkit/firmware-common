#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <cstdint>

namespace fk {

constexpr uint8_t MaximumNumberOfModules = 8;
constexpr uint8_t MaximumNumberOfSensors = 32;

struct SensorInfo {
    uint8_t sensor;
    char name[16];
    char unitOfMeasure[8];
};

enum class SensorReadingStatus {
    Idle,
    Busy,
    Done,
};

struct SensorReading {
    uint8_t sensor;
    uint32_t time;
    float value;
    SensorReadingStatus status;
};

struct ModuleInfo {
    uint8_t address;
    uint8_t numberOfSensors;
    char name[16];
    SensorInfo sensors[MaximumNumberOfSensors];
    SensorReading readings[MaximumNumberOfSensors];
};

}

#endif

