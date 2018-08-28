#ifndef FK_DATA_LOGGING_H_INCLUDED
#define FK_DATA_LOGGING_H_INCLUDED

#include "pool.h"
#include "data_messages.h"
#include "flash_state.h"
#include "files.h"

namespace fk {

class CoreState;

class DataLogging {
private:
    Files *files;

public:
    DataLogging(Files &files);

public:
    bool appendMetadata(CoreState &state);
    bool appendStatus(CoreState &state);
    bool appendLocation(DeviceLocation &location);
    bool appendReading(DeviceLocation &location, uint32_t readingNumber, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading);

private:
    size_t append(DataRecordMessage &message);

};

}

#endif
