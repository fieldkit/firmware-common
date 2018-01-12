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
    uint8_t id;
    SensorInfo &sensor;
    SensorReading &reading;
};

class CoreState {
private:
    NetworkSettings networkSettings;
    ModuleInfo modules[MaximumNumberOfModules];
    DeviceStatus deviceStatus;
    DeviceLocation location;
    DeviceIdentity deviceIdentity;
    FkfsData *data;

public:
    CoreState(FkfsData &data);

public:
    size_t numberOfModules() const;

    size_t numberOfSensors() const;

    size_t numberOfReadings() const;

    ModuleInfo* attachedModules() {
        return modules;
    }

    DeviceLocation &getLocation() {
        return location;
    }

    DeviceIdentity &getIdentity() {
        return deviceIdentity;
    }

    DeviceStatus &getStatus() {
        return deviceStatus;
    }

    NetworkSettings &getNetworkSettings() {
        return networkSettings;
    }

    AvailableSensorReading getReading(size_t index);

public:
    void configure(DeviceIdentity newIdentity);
    void configure(NetworkSettings newSettings);

    void clearReadings();

    void merge(uint8_t address, ModuleReplyMessage &reply);
    void updateBattery(float percentage, float voltage);
    void updateIp(uint32_t ip);
    void updateLocationFixFailed();
    void updateLocation(uint32_t time, float longitude, float latitude, float altitude);

private:
    size_t getModuleIndex(uint8_t address);
    bool appendReading(SensorReading &reading);

};

}

#endif
