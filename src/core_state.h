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

struct PersistedState {
    uint32_t time;
    uint32_t seed;
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };
    fkfs_iterator_token_t cursors[2] { fkfs_token_empty, fkfs_token_empty };

    bool load(FlashStorage &storage);
    bool save(FlashStorage &storage);
};

class CoreState {
private:
    ModuleInfo modules[MaximumNumberOfModules];
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };
    fkfs_iterator_token_t cursors[2] { fkfs_token_empty, fkfs_token_empty };

private:
    DeviceStatus deviceStatus;
    bool busy{ false };
    bool readingInProgress{ false };
    bool wipeAfterUpload{ true };
    FlashStorage *storage;
    FkfsData *data;
    CoreStatus status{ CoreStatus::Initializing };

public:
    CoreState(FlashStorage &storage, FkfsData &data);

public:
    ModuleInfo* attachedModules();
    size_t numberOfModules() const;
    size_t numberOfSensors() const;
    size_t numberOfReadings() const;
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
    bool isReadingInProgress();
    void doneTakingReadings();
    AvailableSensorReading getReading(size_t index);
    void clearReadings();

    void doneScanning();
    void scanFailure();

    void setDeviceId(const char *deviceId);
    void configure(DeviceIdentity newIdentity);
    void configure(NetworkSettings newSettings);

    void merge(uint8_t address, ModuleReplyMessage &reply);
    void merge(uint8_t moduleIndex, IncomingSensorReading &reading);

    void updateBattery(float percentage, float voltage);
    void updateIp(uint32_t ip);
    void updateLocation(DeviceLocation&& fix);

    bool isBusy();
    void setBusy(bool value);
    fkfs_iterator_token_t &getCursor(uint8_t file);
    void saveCursor(fkfs_iterator_token_t &cursor);
    bool shouldWipeAfterUpload();

private:
    size_t getModuleIndex(uint8_t address);
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
