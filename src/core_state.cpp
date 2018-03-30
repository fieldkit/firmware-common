#include <fk-module-protocol.h>

#include "core_state.h"
#include "debug.h"

namespace fk {

bool PersistedState::load(FlashStorage &storage) {
    #ifndef FK_DISABLE_FLASH
    return storage.read(this, sizeof(PersistedState)) == sizeof(PersistedState);
    #else
    return false;
    #endif
}

bool PersistedState::save(FlashStorage &storage) {
    #ifndef FK_DISABLE_FLASH
    return storage.write(this, sizeof(PersistedState)) == sizeof(PersistedState);
    #else
    return false;
    #endif
}

CoreState::CoreState(FlashStorage &storage, FkfsData &data) : storage(&storage), data(&data) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        modules[i].address = 0;
    }
}

void CoreState::started() {
    PersistedState persisted;
    if (persisted.load(*storage)) {
        log("Loaded state.");
        copyFrom(persisted);
    }
    else {
        log("Clean slate!");
        copyTo(persisted);
        persisted.save(*storage);
    }

    data->appendMetadata(*this);
}

void CoreState::doneScanning() {
    data->appendMetadata(*this);
}

void CoreState::scanFailure() {
    memzero(modules, sizeof(modules));
}

void CoreState::merge(uint8_t address, ModuleReplyMessage &reply) {
    auto index = getModuleIndex(address);
    auto& module = modules[index];

    switch (reply.m().type) {
    case fk_module_ReplyType_REPLY_CAPABILITIES: {
        module.address = address;
        module.numberOfSensors = reply.m().capabilities.numberOfSensors;
        strncpy(module.name, (const char *)reply.m().capabilities.name.arg, sizeof(module.name));
        break;
    }
    case fk_module_ReplyType_REPLY_SENSOR_CAPABILITIES: {
        auto sensorIndex = reply.m().sensorCapabilities.id;
        auto& sensor = module.sensors[sensorIndex];
        strncpy(sensor.name, (const char *)reply.m().sensorCapabilities.name.arg, sizeof(sensor.name));
        strncpy(sensor.unitOfMeasure, (const char *)reply.m().sensorCapabilities.unitOfMeasure.arg, sizeof(sensor.unitOfMeasure));
        break;
    }
    case fk_module_ReplyType_REPLY_READING_STATUS: {
        if (reply.m().readingStatus.state == fk_module_ReadingState_DONE) {
            IncomingSensorReading reading{
                (uint8_t)reply.m().sensorReading.sensor,
                reply.m().sensorReading.time,
                reply.m().sensorReading.value,
            };
            merge(index, reading);
        }
        break;
    }
    default:
        break;
    }
}

void CoreState::merge(uint8_t moduleIndex, IncomingSensorReading &incoming) {
    auto& module = modules[moduleIndex];
    auto& reading = module.readings[incoming.sensor];
    auto& sensor = module.sensors[incoming.sensor];
    reading.time = incoming.time;
    reading.value = incoming.value;
    reading.status = SensorReadingStatus::Done;

    if (isTimeOff(reading.time)) {
        auto now = clock.getTime();
        auto difference = abs(now - reading.time);
        debugfpln("CoreState", "Fixing reading with drifted time: %lu - %lu = %d", now, reading.time, difference);
        reading.time = now;
    }

    data->appendReading(location, readingNumber, incoming.sensor, sensor, reading);
}

bool CoreState::hasModuleWithAddress(uint8_t address) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address == address) {
            return true;
        }
    }
    return false;
}

size_t CoreState::getModuleIndex(uint8_t address) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address == 0 || modules[i].address == address) {
            return i;
        }
    }

    fk_assert(false);

    return 0;
}

size_t CoreState::numberOfModules() const {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address == 0) {
            return i;
        }
    }
    return MaximumNumberOfModules;
}

size_t CoreState::numberOfSensors() const {
    size_t number = 0;
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address > 0) {
            number += modules[i].numberOfSensors;
        }
    }
    return number;
}

size_t CoreState::numberOfReadings() const {
    size_t number = 0;
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address > 0) {
            for (size_t j = 0; j < modules[i].numberOfSensors; ++j) {
                if (modules[i].readings[j].status == SensorReadingStatus::Done) {
                    number++;
                }
            }
        }
    }
    return number;
}

AvailableSensorReading CoreState::getReading(size_t index) {
    size_t number = 0;
    for (uint8_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address > 0) {
            for (uint8_t j = 0; j < modules[i].numberOfSensors; ++j) {
                if (modules[i].readings[j].status == SensorReadingStatus::Done) {
                    if (number == index) {
                        return AvailableSensorReading { j, modules[i].sensors[j], modules[i].readings[j] };
                    }
                    number++;
                }
            }
        }
    }

    fk_assert(false);

    return AvailableSensorReading {
        0,
        modules[0].sensors[0],
        modules[0].readings[0]
    };
}

void CoreState::clearReadings() {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        for (size_t j = 0; j < MaximumNumberOfSensors; ++j) {
            modules[i].readings[j].status = SensorReadingStatus::Idle;
        }
    }
}

void CoreState::configure(DeviceIdentity newIdentity) {
    deviceIdentity = newIdentity;
    save();
}

void CoreState::setDeviceId(const char *deviceId) {
    if (deviceIdentity.device[0] == 0) {
        strncpy(deviceIdentity.device, deviceId, MaximumDeviceLength);
    }
}

void CoreState::configure(NetworkSettings newSettings) {
    networkSettings = newSettings;
    networkSettings.version = millis();
    save();
}

void CoreState::updateBattery(float percentage, float voltage) {
    deviceStatus.batteryPercentage = percentage;
    deviceStatus.batteryVoltage = voltage;
}

void CoreState::updateIp(uint32_t ip) {
    deviceStatus.ip = ip;
}

void CoreState::updateLocation(DeviceLocation&& fix) {
    location = fix;
    data->appendLocation(location);
    save();
}

void CoreState::save() {
    PersistedState persisted;
    copyTo(persisted);
    persisted.time = clock.getTime();
    persisted.save(*storage);
    log("Saved");
}

void CoreState::copyFrom(PersistedState &state) {
    deviceIdentity = state.deviceIdentity;
    networkSettings = state.networkSettings;
    location = state.location;
    readingNumber = state.readingNumber;
    cursors[0] = state.cursors[0];
    cursors[1] = state.cursors[1];
}

void CoreState::copyTo(PersistedState &state) {
    state.deviceIdentity = deviceIdentity;
    state.networkSettings = networkSettings;
    state.location = location;
    state.readingNumber = readingNumber;
    state.cursors[0] = cursors[0];
    state.cursors[1] = cursors[1];
}

void CoreState::takingReading() {
    readingNumber++;
    readingInProgress = true;
}

void CoreState::doneTakingReading() {
    readingInProgress = false;
}

bool CoreState::isReadingInProgress() {
    return readingInProgress;
}

bool CoreState::isBusy() {
    return busy;
}

void CoreState::setBusy(bool value) {
    busy = value;
}

fkfs_iterator_token_t &CoreState::getCursor(uint8_t file) {
    return cursors[file];
}

void CoreState::saveCursor(fkfs_iterator_token_t &cursor) {
    cursors[cursor.file] = cursor;
    save();
}

ModuleInfo* CoreState::attachedModules() {
    return modules;
}

DeviceLocation &CoreState::getLocation() {
    return location;
}

DeviceIdentity &CoreState::getIdentity() {
    return deviceIdentity;
}

DeviceStatus &CoreState::getStatus() {
    return deviceStatus;
}

NetworkSettings &CoreState::getNetworkSettings() {
    return networkSettings;
}

bool CoreState::hasModules() {
    return numberOfModules() > 0;
}

bool CoreState::shouldWipeAfterUpload() {
    return wipeAfterUpload;
}

void CoreState::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln(LogLevels::INFO, "CoreState", f, args);
    va_end(args);
}

}
