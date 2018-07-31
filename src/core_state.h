#ifndef FK_CORE_STATE_H_INCLUDED
#define FK_CORE_STATE_H_INCLUDED

#include "app_messages.h"
#include "module_info.h"
#include "module_messages.h"
#include "two_wire_task.h"
#include "fkfs_data.h"
#include "network_settings.h"
#include "flash_storage.h"

namespace fk {

enum class CoreStatus {
    Initializing,
    Ready,
    FatalError,
};

struct PersistedState : phylum::MinimumSuperBlock {
    uint32_t time;
    uint32_t seed;
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };
};

class CoreState {
private:
    ModuleInfo modules[MaximumNumberOfModules];
    ModuleInfo *modulesHead_{ nullptr };
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };

private:
    CoreStatus status{ CoreStatus::Initializing };
    DeviceStatus deviceStatus;
    FlashStorage<PersistedState> *storage;
    FkfsData *data;

public:
    CoreState(FlashStorage<PersistedState> &storage, FkfsData &data);

public:
    ModuleInfo* attachedModules() const;
    size_t numberOfModules() const;
    size_t numberOfModules(fk_module_ModuleType type) const;
    size_t numberOfSensors() const;
    size_t numberOfReadings() const;
    size_t readingsToTake() const;
    bool hasModules();
    bool hasModuleWithAddress(uint8_t address);

    DeviceLocation& getLocation();
    DeviceIdentity& getIdentity();
    DeviceStatus& getStatus();
    NetworkSettings& getNetworkSettings();

public:
    bool isReady() {
        return status == CoreStatus::Ready;
    }
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
    void merge(ModuleInfo &module, IncomingSensorReading &reading);
    ModuleInfo &getModule(uint8_t address);
    ModuleInfo &getOrCreateModule(uint8_t address, uint8_t numberOfSensors);
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
