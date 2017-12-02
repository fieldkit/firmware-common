#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <cstdint>

namespace fk {

struct SensorInfo {
    uint8_t sensor;
    const char *name;
    const char *unitOfMeasure;
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
    const char *name;
    SensorInfo *sensors;
    SensorReading *readings;
};

}

#endif

