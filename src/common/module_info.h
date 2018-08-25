#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <cinttypes>
#include <cstring>

#include <phylum/super_block_manager.h>

#include "network_settings.h"

namespace fk {

constexpr uint8_t MaximumNumberOfModules = 4;
constexpr uint8_t MaximumNumberOfSensors = 32;

struct SensorInfo {
    const char *name;
    const char *unitOfMeasure;
};

enum class SensorReadingStatus {
    Idle,
    Busy,
    Done,
};

struct IncomingSensorReading {
    uint8_t sensor;
    uint32_t time;
    float value;
};

struct SensorReading {
    uint32_t time;
    float value;
    SensorReadingStatus status;
};

struct PendingSensorReading {
    uint32_t number;
    uint32_t elapsed;
    SensorReading *readings;
};

struct ModuleInfo {
    uint8_t type;
    uint8_t address;
    uint8_t numberOfSensors;
    uint8_t minimumNumberOfReadings;
    const char *name;
    const char *module;
    SensorInfo *sensors;
    SensorReading *readings;
    uint32_t compiled;
    ModuleInfo *np;
};

constexpr size_t MaximumCoordinates = 3;
constexpr size_t MaximumDeviceLength = 40 + 1;
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
    uint32_t valid;
    uint32_t time;
    float coordinates[MaximumCoordinates];

    DeviceLocation() : valid(false), time(0), coordinates{ 0.0, 0.0, 0.0 } {
    }

    DeviceLocation(uint32_t time, float longitude, float latitude, float altitude) : valid(true), time(time), coordinates{ longitude, latitude, altitude } {
    }
};

struct AvailableSensorReading {
    uint8_t id;
    SensorInfo sensor;
    SensorReading reading;
};

struct HttpTransmissionConfig {
    const char *streamUrl;
};

enum class FirmwareBank {
    Core,
    CoreNew,
    CoreGood,
    Module,
    ModuleNew,
    ModuleGood,
    NumberOfBanks
};

struct FirmwareAddresses {
    phylum::BlockAddress banks[(size_t)FirmwareBank::NumberOfBanks];
};

struct MinimumFlashState : phylum::MinimumSuperBlock {
    uint32_t time;
    uint32_t seed;
    FirmwareAddresses firmwares;
};

struct PersistedState : MinimumFlashState {
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };
};

}

#endif

