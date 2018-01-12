#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <cstdint>
#include <cstring>

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

struct PendingSensorReading {
    uint32_t elapsed;
    SensorReading *readings;
};

struct ModuleInfo {
    uint8_t address;
    uint8_t numberOfSensors;
    char name[MaximumModuleNameLength];
    SensorInfo sensors[MaximumNumberOfSensors];
    SensorReading readings[MaximumNumberOfSensors];
};

constexpr size_t MaximumCoordinates = 3;
constexpr size_t MaximumDeviceLength = 32 + 1;
constexpr size_t MaximumStreamLength = 8 + 1;

struct DeviceIdentity {
    char device[MaximumDeviceLength];
    char stream[MaximumStreamLength];

    DeviceIdentity() {
        device[0] = 0;
        stream[0] = 0;
    }

    DeviceIdentity(const char *d, const char *s) {
        strncpy(device, d, sizeof(device));
        strncpy(stream, s, sizeof(stream));
    }
};

struct DeviceStatus {
    float batteryPercentage{ 0.0 };
    float batteryVoltage{ 0.0 };
    uint32_t ip{ 0 };

    DeviceStatus() {
    }
};

struct DeviceLocation {
    uint32_t fix;
    uint32_t time;
    float coordinates[MaximumCoordinates];

    DeviceLocation() : time(0), coordinates{ 0.0, 0.0, 0.0 } {
    }
};

}

#endif

