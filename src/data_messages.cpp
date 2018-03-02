#include "data_messages.h"
#include "rtc.h"
#include "core_state.h"
#include "asf.h"

namespace fk {

DataRecordMetadataMessage::DataRecordMetadataMessage(CoreState &state, Pool &pool) : DataRecordMessage(pool) {
    auto *attached = state.attachedModules();
    auto numberOfSensors = state.numberOfSensors();
    auto sensorIndex = 0;
    auto numberOfModules = 0;
    for (size_t moduleIndex = 0; attached[moduleIndex].address > 0; ++moduleIndex) {
        for (size_t i = 0; i < attached[moduleIndex].numberOfSensors; ++i) {
            sensors[sensorIndex].sensor = i;
            sensors[sensorIndex].name.funcs.encode = pb_encode_string;
            sensors[sensorIndex].name.arg = (void *)attached[moduleIndex].sensors[i].name;
            sensors[sensorIndex].unitOfMeasure.funcs.encode = pb_encode_string;
            sensors[sensorIndex].unitOfMeasure.arg = (void *)attached[moduleIndex].sensors[i].unitOfMeasure;

            sensorIndex++;
        }

        modules[numberOfModules].id = numberOfModules;
        modules[numberOfModules].address = attached[moduleIndex].address;
        modules[numberOfModules].name.funcs.encode = pb_encode_string;
        modules[numberOfModules].name.arg = (void *)attached[moduleIndex].name;

        numberOfModules++;
    }

    modulesArray = {
        .length = (size_t)numberOfModules,
        .itemSize = sizeof(fk_data_ModuleInfo),
        .buffer = modules,
        .fields = fk_data_ModuleInfo_fields,
    };

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
    m().metadata.modules.funcs.encode = pb_encode_array;
    m().metadata.modules.arg = (void *)&modulesArray;
}

size_t DataRecordMetadataMessage::calculateSize() {
    size_t size;

    if (!pb_get_encoded_size(&size, fk_data_DataRecord_fields, &m())) {
        return 0;
    }

    return size + ProtoBufEncodeOverhead;
}

}
