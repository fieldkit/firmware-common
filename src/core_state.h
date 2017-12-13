#ifndef FK_CORE_STATE_H_INCLUDED
#define FK_CORE_STATE_H_INCLUDED

#include "app_messages.h"
#include "module_info.h"
#include "module_messages.h"
#include "two_wire_task.h"
#include "fkfs_data.h"
#include "network_settings.h"

namespace fk {

struct AvailableSensorReading {
    uint8_t sensor;
    SensorReading &reading;
};

constexpr size_t MaximumCoordinates = 3;
constexpr size_t MaximumDeviceLength = 16 + 1;
constexpr size_t MaximumStreamLength = 16 + 1;

struct DeviceIdentity {
    char device[MaximumDeviceLength];
    char stream[MaximumStreamLength];

    DeviceIdentity() {
        device[0] = 0;
        stream[0] = 0;
    }
};

class CoreState {
private:
    NetworkSettings networkSettings;
    ModuleInfo modules[MaximumNumberOfModules];
    float coordinates[MaximumCoordinates]{ -118.3604684, 34.0071882, 12.75 };
    DeviceIdentity deviceIdentity;
    FkfsData *data;

public:
    CoreState(FkfsData &data);

public:
    ModuleInfo* attachedModules() {
        return modules;
    }
    size_t numberOfModules() const;
    size_t numberOfSensors() const;
    size_t numberOfReadings() const;
    float *getLocation() {
        return coordinates;
    }
    DeviceIdentity &getIdentity() {
        return deviceIdentity;
    }

public:
    void merge(uint8_t address, ModuleReplyMessage &reply);
    AvailableSensorReading getReading(size_t index);
    void clearReadings();

public:
    void configure(NetworkSettings settings) {
        networkSettings = settings;
        networkSettings.version = millis();
    }
    NetworkSettings &getNetworkSettings() {
        return networkSettings;
    }

private:
    size_t getModuleIndex(uint8_t address);
    bool appendReading(SensorReading &reading);

};

}

#endif
