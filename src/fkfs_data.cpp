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
#include "rtc.h"

namespace fk {

constexpr const char Log[] = "Data";

DataRecordMetadataMessage::DataRecordMetadataMessage(TwoWireBus &bus, CoreState &state, Pool &pool) : DataRecordMessage(pool), deviceId(bus) {
    auto *attached = state.attachedModules();
    auto numberOfSensors = state.numberOfSensors();
    auto sensorIndex = 0;
    for (size_t moduleIndex = 0; attached[moduleIndex].address > 0; ++moduleIndex) {
        for (size_t i = 0; i < attached[moduleIndex].numberOfSensors; ++i) {
            sensors[sensorIndex].sensor = i;
            sensors[sensorIndex].name.funcs.encode = pb_encode_string;
            sensors[sensorIndex].name.arg = (void *)attached[moduleIndex].sensors[i].name;
            sensors[sensorIndex].unitOfMeasure.funcs.encode = pb_encode_string;
            sensors[sensorIndex].unitOfMeasure.arg = (void *)attached[moduleIndex].sensors[i].unitOfMeasure;

            sensorIndex++;
        }
    }

    sensorsArray = {
        .length = numberOfSensors,
        .itemSize = sizeof(fk_data_SensorInfo),
        .buffer = sensors,
        .fields = fk_data_SensorInfo_fields,
    };


    deviceIdData = {
        .length = deviceId.length(),
        .buffer = deviceId.toBuffer(),
    };

    m().metadata.time = clock.getTime();
    m().metadata.resetCause = system_get_reset_cause();
    m().metadata.deviceId.funcs.encode = pb_encode_data;
    m().metadata.deviceId.arg = (void *)&deviceIdData;
    m().metadata.git.funcs.encode = pb_encode_string;
    m().metadata.git.arg = (void *)firmware_version_get();
    m().metadata.sensors.funcs.encode = pb_encode_array;
    m().metadata.sensors.arg = (void *)&sensorsArray;
}

size_t DataRecordMetadataMessage::calculateSize() {
    size_t size;

    if (!pb_get_encoded_size(&size, fk_data_DataRecord_fields, &m())) {
        return 0;
    }

    return size;
}

FkfsData::FkfsData(fkfs_t &fs, TwoWireBus &bus, uint8_t file, Pool &pool) : fs(&fs), bus(&bus), file(file), pool(&pool) {
}

bool FkfsData::appendMetadata(CoreState &state) {
    fk_data_DataRecord record = fk_data_DataRecord_init_default;

    auto *attached = state.attachedModules();
    auto numberOfSensors = state.numberOfSensors();
    auto sensorIndex = 0;
    fk_data_SensorInfo sensors[numberOfSensors];
    for (size_t moduleIndex = 0; attached[moduleIndex].address > 0; ++moduleIndex) {
        for (size_t i = 0; i < attached[moduleIndex].numberOfSensors; ++i) {
            sensors[sensorIndex].sensor = i;
            sensors[sensorIndex].name.funcs.encode = pb_encode_string;
            sensors[sensorIndex].name.arg = (void *)attached[moduleIndex].sensors[i].name;
            sensors[sensorIndex].unitOfMeasure.funcs.encode = pb_encode_string;
            sensors[sensorIndex].unitOfMeasure.arg = (void *)attached[moduleIndex].sensors[i].unitOfMeasure;

            sensorIndex++;
        }
    }

    pb_array_t sensorsArray = {
        .length = numberOfSensors,
        .itemSize = sizeof(fk_data_SensorInfo),
        .buffer = sensors,
        .fields = fk_data_SensorInfo_fields,
    };


    DeviceId deviceId{ *bus };
    pb_data_t deviceIdData = {
        .length = deviceId.length(),
        .buffer = deviceId.toBuffer(),
    };

    record.metadata.time = clock.getTime();
    record.metadata.resetCause = system_get_reset_cause();
    record.metadata.deviceId.funcs.encode = pb_encode_data;
    record.metadata.deviceId.arg = (void *)&deviceIdData;
    record.metadata.git.funcs.encode = pb_encode_string;
    record.metadata.git.arg = (void *)firmware_version_get();
    record.metadata.sensors.funcs.encode = pb_encode_array;
    record.metadata.sensors.arg = (void *)&sensorsArray;

    auto size = append(record);

    log("Appended metadata (%d bytes)", size);

    return true;
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

    log("Appended location (%d bytes)", size);

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

    log("Appended reading (%d bytes) (%lu, %d, '%s' = %f)", size, reading.time, sensorId, sensor.name, reading.value);

    return true;
}

size_t FkfsData::append(fk_data_DataRecord &record) {
    size_t size;

    if (!pb_get_encoded_size(&size, fk_data_DataRecord_fields, &record)) {
        return false;
    }

    uint8_t buffer[size + ProtoBufEncodeOverhead];
    auto stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode_delimited(&stream, fk_data_DataRecord_fields, &record)) {
        log("Error encoding data file record (%d/%d bytes)", size, sizeof(buffer));
        return 0;
    }

    if (!fkfs_file_append(fs, file, stream.bytes_written, buffer)) {
        log("Error appending data file.");
        return 0;
    }

    return stream.bytes_written;
}

void FkfsData::log(const char *f, ...) const {
    va_list args;
    va_start(args, f);
    vdebugfpln(Log, f, args);
    va_end(args);
}

}
