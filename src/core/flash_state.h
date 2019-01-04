#ifndef FK_FLASH_STATE_H_INCLUDED
#define FK_FLASH_STATE_H_INCLUDED

#include "module_info.h"
#include "network_settings.h"

namespace fk {

constexpr size_t MaximumCoordinates = 3;
constexpr size_t MaximumDeviceLength = 40 + 1;
constexpr size_t MaximumStreamLength = 8 + 1;

struct DeviceIdentity {
    char device[MaximumDeviceLength];
    char stream[MaximumStreamLength];

    DeviceIdentity() {
        device[0] = 0;
        stream[0] = 0;
    }

    DeviceIdentity(const char *d, const char *s) {
        strncpy(device, d, sizeof(device));
        strncpy(stream, s, sizeof(stream));
    }
};

struct BatteryStatus {
    bool attached;
    bool low;
    bool ok;
    float voltage;
    float coulombs;
    float delta;
    float percentage;
    float ma;

    bool charging() const {
        return ma > 0.0f;
    }
};

struct DeviceStatus {
    BatteryStatus battery;
    uint32_t ip{ 0 };

    DeviceStatus() {
    }
};

struct DeviceLocation {
    uint32_t valid;
    uint32_t time;
    float coordinates[MaximumCoordinates];

    DeviceLocation() : valid(false), time(0), coordinates{ 0.0, 0.0, 0.0 } {
    }

    DeviceLocation(uint32_t time, float longitude, float latitude, float altitude) : valid(true), time(time), coordinates{ longitude, latitude, altitude } {
    }
};

struct PersistedState : MinimumFlashState {
    DeviceIdentity deviceIdentity;
    NetworkSettings networkSettings;
    DeviceLocation location;
    uint32_t readingNumber{ 0 };
};

struct HttpTransmissionConfig {
    const char *streamUrl;
};

struct AvailableSensorReading {
    uint8_t id;
    SensorInfo sensor;
    SensorReading reading;
};

}

#endif
