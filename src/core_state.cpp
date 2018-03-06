#include <fk-module-protocol.h>

#include "core_state.h"
#include "debug.h"

namespace fk {

CoreState::CoreState(FkfsData &data) : data(&data) {
    for (size_t i = 0; i < MaximumNumberOfModules; ++i) {
        modules[i].address = 0;
    }
}

void CoreState::started() {
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

    // Try and help modules that don't have accurate clocks.
    if (reading.time == 0) {
        reading.time = clock.getTime();
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
}

void CoreState::setDeviceId(const char *deviceId) {
    if (deviceIdentity.device[0] == 0) {
        strncpy(deviceIdentity.device, deviceId, MaximumDeviceLength);
    }
}

void CoreState::configure(NetworkSettings newSettings) {
    networkSettings = newSettings;
    networkSettings.version = millis();
}

void CoreState::updateBattery(float percentage, float voltage) {
    deviceStatus.batteryPercentage = percentage;
    deviceStatus.batteryVoltage = voltage;
}

void CoreState::updateIp(uint32_t ip) {
    deviceStatus.ip = ip;
}

void CoreState::updateLocationFixFailed() {
    location.fix = false;
}

void CoreState::updateLocation(uint32_t time, float longitude, float latitude, float altitude) {
    location.fix = true;
    location.time = time;
    location.coordinates[0] = longitude;
    location.coordinates[1] = latitude;
    location.coordinates[2] = altitude;

    data->appendLocation(location);
}

}
