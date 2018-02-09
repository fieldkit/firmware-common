#ifndef FK_FKFS_DATA_H_INCLUDED
#define FK_FKFS_DATA_H_INCLUDED

#include <fk-data-protocol.h>

#include <fkfs.h>

#include "module_info.h"
#include "pool.h"
#include "i2c.h"

#include "device_id.h"
#include "protobuf.h"

namespace fk {

class CoreState;

class DataRecordMessage {
private:
    fk_data_DataRecord message = fk_data_DataRecord_init_default;
    Pool *pool;

public:
    DataRecordMessage(Pool &pool) : pool(&pool) {
    }

    void clear() {
        message = fk_data_DataRecord_init_default;
    }

    fk_data_DataRecord *forDecode() {
        return &message;
    }

    fk_data_DataRecord *forEncode() {
        return &message;
    }

    fk_data_DataRecord &m() {
        return message;
    }

};

class DataRecordMetadataMessage : public DataRecordMessage {
private:
    DeviceId deviceId;
    fk_data_SensorInfo sensors[MaximumNumberOfSensors];
    pb_data_t deviceIdData;
    pb_array_t sensorsArray;

public:
    DataRecordMetadataMessage(TwoWireBus &bus, CoreState &state, Pool &pool);

public:
    size_t calculateSize();

};

class FkfsData {
private:
    fkfs_t *fs;
    TwoWireBus *bus;
    uint8_t file;
    Pool *pool;

public:
    FkfsData(fkfs_t &fs, TwoWireBus &bus, uint8_t file, Pool &pool);

public:
    bool appendMetadata(CoreState &state);
    bool appendLocation(DeviceLocation &location);
    bool appendReading(DeviceLocation &location, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading);

private:
    size_t append(fk_data_DataRecord &record);
    void log(const char *f, ...) const;

};

}

#endif
