#ifndef FK_MODULE_INFO_H_INCLUDED
#define FK_MODULE_INFO_H_INCLUDED

#include <phylum/super_block_manager.h>

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

}

#endif

