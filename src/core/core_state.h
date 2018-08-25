#ifndef FK_CORE_STATE_H_INCLUDED
#define FK_CORE_STATE_H_INCLUDED

#include <phylum/private.h>

#include "app_messages.h"
#include "module_info.h"
#include "module_messages.h"
#include "two_wire_task.h"
#include "fkfs_data.h"
#include "flash_storage.h"

namespace fk {

class CoreState {
private:
    StaticPool<2048> pool_{ "CoreState" };
    ModuleInfo *modules_{ nullptr };
    DeviceIdentity deviceIdentity_;
    NetworkSettings networkSettings_;
    DeviceLocation location_;
    uint32_t readingNumber_{ 0 };

private:
    DeviceStatus deviceStatus_;
    FlashState<PersistedState> *storage_;
    FkfsData *data_;

public:
    CoreState(FlashState<PersistedState> &storage, FkfsData &data);

public:
    ModuleInfo* attachedModules() const;
    size_t numberOfModules() const;
    size_t numberOfModules(fk_module_ModuleType type) const;
    size_t numberOfSensors() const;
    size_t numberOfReadings() const;
    size_t readingsToTake() const;
    ModuleInfo *getModuleByIndex(uint8_t index);
    ModuleInfo *getModule(uint8_t address);
    bool hasModules();
    bool hasModuleWithAddress(uint8_t address);
    void merge(ModuleInfo &module, IncomingSensorReading &reading);

    DeviceLocation& getLocation();
    DeviceIdentity& getIdentity();
    DeviceStatus& getStatus();
    NetworkSettings& getNetworkSettings();

public:
    void started();

    void takingReadings();
    void doneTakingReadings();
    AvailableSensorReading getReading(size_t index);
    void clearReadings();

    void doneScanning();
    void scanFailure();

    void setDeviceId(const char *deviceId);
    void configure(DeviceIdentity newIdentity);
    void configure(NetworkSettings newSettings);

    void merge(uint8_t address, ModuleReplyMessage &reply);

    void updateBattery(float percentage, float voltage);
    void updateIp(uint32_t ip);
    void updateLocation(DeviceLocation&& fix);

    void formatAll();

private:
    ModuleInfo *getOrCreateModule(uint8_t address, uint8_t numberOfSensors);
    bool appendReading(SensorReading &reading);

private:
    void copyFrom(PersistedState &state);
    void copyTo(PersistedState &state);
    void save();

private:
    void log(const char *f, ...) const;

};

}

#endif
