#ifndef FK_FKFS_DATA_H_INCLUDED
#define FK_FKFS_DATA_H_INCLUDED

#include <fk-data-protocol.h>

#include <fkfs.h>

#include "module_info.h"

namespace fk {

class FkfsData {
private:
    fkfs_t *fs;
    uint8_t file;

public:
    FkfsData(fkfs_t &fs, uint8_t file);

public:
    bool appendLocation(DeviceLocation &location);
    bool appendReading(DeviceLocation &location, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading);

private:
    size_t append(fk_data_DataRecord &record);

};

}

#endif
