#include <fk-module-protocol.h>

#include "core_state.h"
#include "debug.h"

namespace fk {

CoreState::CoreState(FlashStorage<PersistedState> &storage, FkfsData &data) : storage(&storage), data(&data) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        modules[i].address = 0;
    }
    modulesHead_ = nullptr;
}

void CoreState::started() {
    // modulesHead_ = nullptr;
    // Free

    auto &persisted = storage->state();
    if (persisted.time == 0) {
        log("Clean slate!");
        copyTo(persisted);
    }
    else {
        log("Loaded state.");
        copyFrom(persisted);
    }

    log("Metadata");
    data->appendMetadata(*this);
    log("Status");
    data->appendStatus(*this);

    log("Started");
}

void CoreState::formatAll() {
    if (!storage->erase()) {
        log("Flash storage erase failed.");
    }
}

void CoreState::doneScanning() {
    data->appendMetadata(*this);
    data->appendStatus(*this);
    status = CoreStatus::Ready;
}

void CoreState::scanFailure() {
    fk_memzero(modules, sizeof(modules));
    // modulesHead_ = nullptr;
    // Free
}

void CoreState::merge(uint8_t address, ModuleReplyMessage &reply) {
    switch (reply.m().type) {
    case fk_module_ReplyType_REPLY_CAPABILITIES: {
        log("Caps");
        auto& module = getOrCreateModule(address, reply.m().capabilities.numberOfSensors);
        module.address = address;
        module.type = reply.m().capabilities.type;
        module.numberOfSensors = reply.m().capabilities.numberOfSensors;
        module.minimumNumberOfReadings = reply.m().capabilities.minimumNumberOfReadings;
        strncpy(module.name, (const char *)reply.m().capabilities.name.arg, sizeof(module.name));
        strncpy(module.module, (const char *)reply.m().capabilities.module.arg, sizeof(module.module));
        break;
    }
    case fk_module_ReplyType_REPLY_SENSOR_CAPABILITIES: {
        auto& module = getModule(address);
        auto sensorIndex = reply.m().sensorCapabilities.id;
        auto& sensor = module.sensors[sensorIndex];
        strncpy(sensor.name, (const char *)reply.m().sensorCapabilities.name.arg, sizeof(sensor.name));
        strncpy(sensor.unitOfMeasure, (const char *)reply.m().sensorCapabilities.unitOfMeasure.arg, sizeof(sensor.unitOfMeasure));
        break;
    }
    case fk_module_ReplyType_REPLY_READING_STATUS: {
        auto& module = getModule(address);
        if (reply.m().readingStatus.state == fk_module_ReadingState_DONE) {
            IncomingSensorReading reading{
                (uint8_t)reply.m().sensorReading.sensor,
                reply.m().sensorReading.time,
                reply.m().sensorReading.value,
            };
            merge(module, reading);
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

    data->appendReading(location, readingNumber, incoming.sensor, sensor, reading);
}

bool CoreState::hasModuleWithAddress(uint8_t address) {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->address == address) {
            return true;
        }
    }
    return false;
}

ModuleInfo &CoreState::getOrCreateModule(uint8_t address, uint8_t numberOfSensors) {
    return getModule(address);
}

ModuleInfo &CoreState::getModule(uint8_t address) {
    auto tail = (ModuleInfo *)nullptr;

    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        if (m->address == address) {
            return *m;
        }
        tail = m;
    }

    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        if (modules[i].address == 0) {
            if (tail == nullptr) {
                modulesHead_ = &modules[i];
            }
            else {
                tail->np = &modules[i];
            }
            modules[i].np = nullptr;
            return modules[i];
        }
    }

    fk_assert(false);

    return modules[0]; // TODO: Remove
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

    return AvailableSensorReading {
        0,
        modules[0].sensors[0],
        modules[0].readings[0]
    };
}

void CoreState::clearReadings() {
    for (auto m = attachedModules(); m != nullptr; m = m->np) {
        for (size_t j = 0; j < m->numberOfSensors; ++j) {
            m->readings[j].status = SensorReadingStatus::Idle;
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
    networkSettings.version = fk_uptime();
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
    data->appendStatus(*this);
    save();
}

void CoreState::save() {
    auto &persisted = storage->state();
    copyTo(persisted);
    persisted.time = clock.getTime();
    storage->save();
}

void CoreState::copyFrom(PersistedState &state) {
    deviceIdentity = state.deviceIdentity;
    networkSettings = state.networkSettings;
    location = state.location;
    readingNumber = state.readingNumber;
}

void CoreState::copyTo(PersistedState &state) {
    state.deviceIdentity = deviceIdentity;
    state.networkSettings = networkSettings;
    state.location = location;
    state.readingNumber = readingNumber;
}

void CoreState::takingReadings() {
    readingNumber++;
}

void CoreState::doneTakingReadings() {
    data->doneTakingReadings();
}

ModuleInfo* CoreState::attachedModules() const {
    return modulesHead_;
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

void CoreState::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, "CoreState", f, args);
    va_end(args);
}

}
