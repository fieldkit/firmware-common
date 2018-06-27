#include <../Adafruit_ASFcore/reset.h>
#undef min
#undef max
#undef HIGH
#undef LOW

#include "fkfs_data.h"
#include "core_state.h"
#include "debug.h"
#include "device_id.h"
#include "protobuf.h"

namespace fk {

constexpr const char Log[] = "Data";

FkfsData::FkfsData(TwoWireBus &bus, Files &files, Pool &pool) : bus(&bus), files(&files), pool(&pool) {
}

bool FkfsData::appendMetadata(CoreState &state) {
    DataRecordMetadataMessage message{ state, *pool };

    auto size = append(message);

    log("Appended metadata (%d bytes)", size);

    return true;
}

bool FkfsData::appendStatus(CoreState &state) {
    DataRecordStatusMessage message{ state, *pool };

    auto size = append(message);

    log("Appended status (%d bytes)", size);

    return true;
}

bool FkfsData::appendLocation(DeviceLocation &location) {
    DataRecordMessage message{ *pool };

    message.m().loggedReading.version = 1;
    message.m().loggedReading.location.fix = location.valid;
    message.m().loggedReading.location.time = location.time;
    message.m().loggedReading.location.longitude = location.coordinates[0];
    message.m().loggedReading.location.latitude = location.coordinates[1];
    message.m().loggedReading.location.altitude = location.coordinates[2];

    auto size = append(message);

    log("Appended location (%d bytes)", size);

    return true;
}

bool FkfsData::appendReading(DeviceLocation &location, uint32_t readingNumber, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading) {
    DataRecordMessage message{ *pool };

    message.m().loggedReading.version = 1;
    message.m().loggedReading.location.fix = location.valid;
    message.m().loggedReading.location.time = location.time;
    message.m().loggedReading.location.longitude = location.coordinates[0];
    message.m().loggedReading.location.latitude = location.coordinates[1];
    message.m().loggedReading.location.altitude = location.coordinates[2];
    message.m().loggedReading.reading.reading = readingNumber;
    message.m().loggedReading.reading.time = reading.time;
    message.m().loggedReading.reading.sensor = sensorId;
    message.m().loggedReading.reading.value = reading.value;

    auto size = append(message);

    log("Appended reading (%d bytes) (%lu, %d, '%s' = %f)", size, reading.time, sensorId, sensor.name, reading.value);

    return true;
}

size_t FkfsData::append(DataRecordMessage &message) {
    size_t size;

    if (!pb_get_encoded_size(&size, fk_data_DataRecord_fields, message.forEncode())) {
        return false;
    }

    uint8_t buffer[size + ProtoBufEncodeOverhead];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, message.forEncode())) {
        log("Error encoding data file record (%d/%d bytes)", size, sizeof(buffer));
        return 0;
    }

    int32_t bytes = stream.bytes_written;
    if (files->data().write(buffer, bytes, true) != bytes) {
        log("Error appending data file.");
        return 0;
    }

    return stream.bytes_written;
}

bool FkfsData::doneTakingReadings() {
    return true;
}

void FkfsData::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vlogf(LogLevels::INFO, Log, f, args);
    va_end(args);
}

}
