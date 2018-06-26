#ifndef FK_FKFS_DATA_H_INCLUDED
#define FK_FKFS_DATA_H_INCLUDED

#include <fk-data-protocol.h>

#include "module_info.h"
#include "pool.h"
#include "two_wire.h"
#include "data_messages.h"

namespace fk {

class CoreState;

class FkfsData {
private:
    TwoWireBus *bus;
    Pool *pool;

public:
    FkfsData(TwoWireBus &bus, Pool &pool);

public:
    bool appendMetadata(CoreState &state);
    bool appendStatus(CoreState &state);
    bool appendLocation(DeviceLocation &location);
    bool appendReading(DeviceLocation &location, uint32_t readingNumber, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading);
    bool doneTakingReadings();

private:
    size_t append(DataRecordMessage &message);

private:
    void log(const char *f, ...) const;

};

}

#endif
