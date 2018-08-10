#include <fk-module-protocol.h>

#include "core_state.h"
#include "debug.h"

namespace fk {

CoreState::CoreState(FlashStorage<PersistedState> &storage, FkfsData &data) : storage_(&storage), data_(&data) {
    modules_ = nullptr;
}

void CoreState::started() {
    pool_.clear();
    modules_ = nullptr;

    auto &persisted = storage_->state();
    if (persisted.time == 0) {
        log("Clean slate!");
        copyTo(persisted);
    }
    else {
        log("Loaded state.");
        copyFrom(persisted);
    }

    data_->appendMetadata(*this);
    data_->appendStatus(*this);
}

void CoreState::formatAll() {
    if (!storage_->erase()) {
        log("Flash storage erase failed.");
    }
}

void CoreState::doneScanning() {
    log("Scan done (%d bytes)", pool_.allocated());

    data_->appendMetadata(*this);
    data_->appendStatus(*this);
}

void CoreState::scanFailure() {
    pool_.clear();
    modules_ = nullptr;
}

void CoreState::merge(uint8_t address, ModuleReplyMessage &reply) {
    switch (reply.m().type) {
    case fk_module_ReplyType_REPLY_CAPABILITIES: {
        auto module = getOrCreateModule(address, reply.m().capabilities.numberOfSensors);
        module->address = address;
        module->type = reply.m().capabilities.type;
        module->numberOfSensors = reply.m().capabilities.numberOfSensors;
        module->minimumNumberOfReadings = reply.m().capabilities.minimumNumberOfReadings;
        module->name = pool_.strdup((const char *)reply.m().capabilities.name.arg);
        module->module = pool_.strdup((const char *)reply.m().capabilities.module.arg);
        break;
    }
    case fk_module_ReplyType_REPLY_SENSOR_CAPABILITIES: {
        auto module = getModule(address);
        auto sensorIndex = reply.m().sensorCapabilities.id;
        auto& sensor = module->sensors[sensorIndex];
        sensor.name = pool_.strdup((const char *)reply.m().sensorCapabilities.name.arg);
        sensor.unitOfMeasure = pool_.strdup((const char *)reply.m().sensorCapabilities.unitOfMeasure.arg);
        break;
    }
    case fk_module_ReplyType_REPLY_READING_STATUS: {
        auto module = getModule(address);
        if (reply.m().readingStatus.state == fk_module_ReadingState_DONE) {
            IncomingSensorReading reading{
                (uint8_t)reply.m().sensorReading.sensor,
                reply.m().sensorReading.time,
                reply.m().sensorReading.value,
            };
            merge(*module, reading);
        }
        break;
    }
    default:
        break;
    }
}

void CoreState::merge(ModuleInfo &module, IncomingSensorReading &incoming) {
    auto& reading = module.readings[incoming.sensor];
    auto& sensor = module.sensors[incoming.sensor];
    reading.time = incoming.time;
    reading.value = incoming.value;
    reading.status = SensorReadingStatus::Done;

    if (isTimeOff(reading.time)) {
        auto now = clock.getTime();
        auto difference = abs(now - reading.time);
        log("Fixing reading with drifted time: %lu - %lu = %d", now, reading.time, difference);
        reading.time = now;
    }

    data_->appendReading(location_, readingNumber_, incoming.sensor, sensor, reading);
}

bool CoreState::hasModuleWithAddress(uint8_t address) {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->address == address) {
            return true;
        }
    }
    return false;
}

ModuleInfo *CoreState::getOrCreateModule(uint8_t address, uint8_t numberOfSensors) {
    ModuleInfo *tail = nullptr;

    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->address == address) {
            return m;
        }
        tail = m;
    }

    auto module = (ModuleInfo *)pool_.malloc(sizeof(ModuleInfo));
    memset(module, 0, sizeof(ModuleInfo));
    module->sensors = (SensorInfo *)pool_.malloc(sizeof(SensorInfo) * numberOfSensors);
    module->readings = (SensorReading *)pool_.malloc(sizeof(SensorReading) * numberOfSensors);
    if (tail == nullptr) {
        modules_ = module;
    }
    else {
        tail->np = module;
    }

    return module;
}

ModuleInfo *CoreState::getModuleByIndex(uint8_t index) {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (index == 0) {
            return m;
        }
        index--;
    }
    fk_assert(false);
    return nullptr;
}

ModuleInfo *CoreState::getModule(uint8_t address) {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->address == address) {
            return m;
        }
    }
    fk_assert(false);
    return nullptr;
}

size_t CoreState::numberOfModules() const {
    size_t number = 0;
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        number++;
    }
    return number;
}

size_t CoreState::numberOfModules(fk_module_ModuleType type) const {
    size_t number = 0;
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->type == type) {
            number++;
        }
    }
    return number;
}

size_t CoreState::numberOfSensors() const {
    size_t number = 0;
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        number += m->numberOfSensors;
    }
    return number;
}

size_t CoreState::numberOfReadings() const {
    size_t number = 0;
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        for (size_t j = 0; j < m->numberOfSensors; ++j) {
            if (m->readings[j].status == SensorReadingStatus::Done) {
                number++;
            }
        }
    }
    return number;
}

size_t CoreState::readingsToTake() const {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->type == fk_module_ModuleType_SENSOR) {
            // NOTE: We only support one sensor module right now.
            return m->minimumNumberOfReadings;
        }
    }
    return 1;
}

AvailableSensorReading CoreState::getReading(size_t index) {
    size_t number = 0;
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        for (uint8_t j = 0; j < m->numberOfSensors; ++j) {
            if (m->readings[j].status == SensorReadingStatus::Done) {
                if (number == index) {
                    return AvailableSensorReading { j, m->sensors[j], m->readings[j] };
                }
                number++;
            }
        }
    }

    fk_assert(false);

    return AvailableSensorReading { };
}

void CoreState::clearReadings() {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        for (size_t j = 0; j < m->numberOfSensors; ++j) {
            m->readings[j].status = SensorReadingStatus::Idle;
        }
    }
}

void CoreState::configure(DeviceIdentity newIdentity) {
    deviceIdentity_ = newIdentity;
    save();
}

void CoreState::setDeviceId(const char *deviceId) {
    if (deviceIdentity_.device[0] == 0) {
        strncpy(deviceIdentity_.device, deviceId, MaximumDeviceLength);
    }
}

void CoreState::configure(NetworkSettings newSettings) {
    networkSettings_ = newSettings;
    networkSettings_.version = fk_uptime();
    save();
}

void CoreState::updateBattery(float percentage, float voltage) {
    deviceStatus_.batteryPercentage = percentage;
    deviceStatus_.batteryVoltage = voltage;
}

void CoreState::updateIp(uint32_t ip) {
    deviceStatus_.ip = ip;
}

void CoreState::updateLocation(DeviceLocation&& fix) {
    location_ = fix;
    data_->appendLocation(location_);
    data_->appendStatus(*this);
    save();
}

void CoreState::save() {
    auto &persisted = storage_->state();
    copyTo(persisted);
    persisted.time = clock.getTime();
    storage_->save();
}

void CoreState::copyFrom(PersistedState &state) {
    deviceIdentity_ = state.deviceIdentity;
    networkSettings_ = state.networkSettings;
    location_ = state.location;
    readingNumber_ = state.readingNumber;
}

void CoreState::copyTo(PersistedState &state) {
    state.deviceIdentity = deviceIdentity_;
    state.networkSettings = networkSettings_;
    state.location = location_;
    state.readingNumber = readingNumber_;
}

void CoreState::takingReadings() {
    readingNumber_++;
}

void CoreState::doneTakingReadings() {
    data_->doneTakingReadings();
}

ModuleInfo* CoreState::attachedModules() const {
    return modules_;
}

DeviceLocation &CoreState::getLocation() {
    return location_;
}

DeviceIdentity &CoreState::getIdentity() {
    return deviceIdentity_;
}

DeviceStatus &CoreState::getStatus() {
    return deviceStatus_;
}

NetworkSettings &CoreState::getNetworkSettings() {
    return networkSettings_;
}

bool CoreState::hasModules() {
    return numberOfModules() > 0;
}

void CoreState::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "CoreState", f, args);
    va_end(args);
}

}
