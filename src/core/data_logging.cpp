#include <../Adafruit_ASFcore/reset.h>
#undef min
#undef max
#undef HIGH
#undef LOW

#include "data_logging.h"
#include "core_state.h"
#include "debug.h"
#include "device_id.h"
#include "protobuf.h"

namespace fk {

constexpr const char Log[] = "Data";

using Logger = SimpleLog<Log>;

DataLogging::DataLogging(Files &files) : files(&files) {
}

bool DataLogging::appendMetadata(CoreState &state) {
    EmptyPool pool;
    DataRecordMetadataMessage message{ state, pool };

    auto size = append(message);

    Logger::info("Appended metadata (%d bytes)", size);

    return true;
}

bool DataLogging::appendStatus(CoreState &state) {
    if (!appendMetadataIfNecessary(state)) {
        return false;
    }

    EmptyPool pool;
    DataRecordStatusMessage message{ state, pool };

    auto size = append(message);

    Logger::info("Appended status (%d bytes)", size);

    return true;
}

bool DataLogging::appendLocation(CoreState &state, DeviceLocation &location) {
    if (!appendMetadataIfNecessary(state)) {
        return false;
    }

    EmptyPool pool;
    DataRecordMessage message{ pool };

    message.m().loggedReading.version = 1;
    message.m().loggedReading.location.fix = location.valid;
    message.m().loggedReading.location.time = location.time;
    message.m().loggedReading.location.longitude = location.coordinates[0];
    message.m().loggedReading.location.latitude = location.coordinates[1];
    message.m().loggedReading.location.altitude = location.coordinates[2];

    auto time = clock.getTime();

    message.m().status.time = time;
    message.m().status.uptime = fk_uptime();
    message.m().status.battery = 0.0f;
    message.m().status.memory = 0;
    message.m().status.busy = 0;

    auto size = append(message);

    Logger::info("Appended location (%d bytes)", size);

    return true;
}

bool DataLogging::appendReading(CoreState &state, DeviceLocation &location, uint32_t readingNumber, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading) {
    if (!appendMetadataIfNecessary(state)) {
        return false;
    }

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

    auto time = clock.getTime();

    message.m().status.time = time;
    message.m().status.uptime = fk_uptime();
    message.m().status.battery = 0.0f;
    message.m().status.memory = 0;
    message.m().status.busy = 0;

    auto size = append(message);

    Logger::info("Appended reading (%d bytes) (%lu, %lu, '%s' = %f)", size, reading.time, sensorId, sensor.name, reading.value);

    return true;
}

bool DataLogging::appendMetadataIfNecessary(CoreState &state) {
    if (files->data().tell() > 0) {
        return true;
    }

    return appendMetadata(state);
}

size_t DataLogging::append(DataRecordMessage &message) {
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

}
