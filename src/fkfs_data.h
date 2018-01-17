#ifndef FK_FKFS_DATA_H_INCLUDED
#define FK_FKFS_DATA_H_INCLUDED

#include <fkfs.h>

#include "module_info.h"

namespace fk {

struct LoggedSensorReading {
    uint32_t time{ 0 };
    uint32_t sensor{ 0 };
    float value{ 0.0f };

    LoggedSensorReading() {
    }

    LoggedSensorReading(uint32_t sensor, SensorReading reading) : time(reading.time), sensor(sensor), value(reading.value) {
    }
};

struct DataEntry {
    uint32_t version{ 0 };
    DeviceLocation location;
    LoggedSensorReading reading;

    DataEntry(DeviceLocation location) : location(location) {
    }

    DataEntry(DeviceLocation location, LoggedSensorReading reading) : location(location), reading(reading) {
    }
};

class FkfsData {
private:
    fkfs_t *fs;
    uint8_t file;

public:
    FkfsData(fkfs_t &fs, uint8_t file);

public:
    bool appendLocation(DeviceLocation &location);
    bool appendReading(DeviceLocation &location, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading);

};

}

#endif
