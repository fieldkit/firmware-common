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

using Logger = SimpleLog<Log>;

FkfsData::FkfsData(Files &files) : files(&files) {
}

bool FkfsData::appendMetadata(CoreState &state) {
    EmptyPool pool;
    DataRecordMetadataMessage message{ state, pool };

    auto size = append(message);

    Logger::info("Appended metadata (%d bytes)", size);

    return true;
}

bool FkfsData::appendStatus(CoreState &state) {
    EmptyPool pool;
    DataRecordStatusMessage message{ state, pool };

    auto size = append(message);

    Logger::info("Appended status (%d bytes)", size);

    return true;
}

bool FkfsData::appendLocation(DeviceLocation &location) {
    EmptyPool pool;
    DataRecordMessage message{ pool };

    message.m().loggedReading.version = 1;
    message.m().loggedReading.location.fix = location.valid;
    message.m().loggedReading.location.time = location.time;
    message.m().loggedReading.location.longitude = location.coordinates[0];
    message.m().loggedReading.location.latitude = location.coordinates[1];
    message.m().loggedReading.location.altitude = location.coordinates[2];

    auto size = append(message);

    Logger::info("Appended location (%d bytes)", size);

    return true;
}

bool FkfsData::appendReading(DeviceLocation &location, uint32_t readingNumber, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading) {
    EmptyPool pool;
    DataRecordMessage message{ pool };

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

    Logger::info("Appended reading (%d bytes) (%lu, %lu, '%s' = %f)", size, reading.time, sensorId, sensor.name, reading.value);

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
        Logger::error("Error encoding data file record (%d/%d bytes)", size, sizeof(buffer));
        return 0;
    }

    auto bytes = stream.bytes_written;
    auto written = (uint32_t)files->data().write(buffer, bytes, true);
    if (written != bytes) {
        Logger::error("Error appending data file (%d != %lu).", bytes, written);
        return 0;
    }

    return stream.bytes_written;
}

bool FkfsData::doneTakingReadings() {
    return true;
}

}
