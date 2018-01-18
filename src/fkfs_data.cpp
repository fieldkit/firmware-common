#include "fkfs_data.h"

#include "debug.h"

namespace fk {

constexpr const char Log[] = "Data";

FkfsData::FkfsData(fkfs_t &fs, uint8_t file) : fs(&fs), file(file) {
}

size_t FkfsData::append(fk_data_DataRecord &record) {
    size_t size;

    if (!pb_get_encoded_size(&size, fk_data_DataRecord_fields, &record)) {
        return false;
    }

    uint8_t buffer[FK_DATA_PROTOCOL_MAX_DATA_MESSAGE];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, &record)) {
        debugfpln(Log, "Stream needs %d, we have %d", size, sizeof(buffer));
        return 0;
    }

    if (!fkfs_file_append(fs, file, stream.bytes_written, buffer)) {
        debugfpln(Log, "Error appending data file.");
        return 0;
    }

    return stream.bytes_written;
}

bool FkfsData::appendLocation(DeviceLocation &location) {
    fk_data_DataRecord record = fk_data_DataRecord_init_default;

    record.loggedReading.version = 1;
    record.loggedReading.location.fix = location.fix;
    record.loggedReading.location.time = location.time;
    record.loggedReading.location.longitude = location.coordinates[0];
    record.loggedReading.location.latitude = location.coordinates[1];
    record.loggedReading.location.altitude = location.coordinates[2];

    auto size = append(record);

    debugfpln(Log, "Appended location (%d bytes)", size);

    return true;
}

bool FkfsData::appendReading(DeviceLocation &location, uint32_t sensorId, SensorInfo &sensor, SensorReading &reading) {
    fk_data_DataRecord record = fk_data_DataRecord_init_default;

    record.loggedReading.version = 1;
    record.loggedReading.location.fix = location.fix;
    record.loggedReading.location.time = location.time;
    record.loggedReading.location.longitude = location.coordinates[0];
    record.loggedReading.location.latitude = location.coordinates[1];
    record.loggedReading.location.altitude = location.coordinates[2];
    record.loggedReading.reading.time = reading.time;
    record.loggedReading.reading.sensor = sensorId;
    record.loggedReading.reading.value = reading.value;

    auto size = append(record);

    debugfpln(Log, "Appended reading (%d bytes) (%lu, '%s' = %f)", size, reading.time, sensor.name, reading.value);

    return true;
}

}
