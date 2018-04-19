#ifndef FK_FKFS_DATA_MESSAGES_H_INCLUDED
#define FK_FKFS_DATA_MESSAGES_H_INCLUDED

#include <fk-data-protocol.h>

#include "module_info.h"
#include "pool.h"
#include "two_wire.h"
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

public:
    size_t calculateSize();

};

class DataLogMessage : public DataRecordMessage {
public:
    DataLogMessage(const fk_log_message_t *raw, Pool &pool) : DataRecordMessage(pool) {
        m().log.uptime = raw->uptime;
        m().log.time = raw->time;
        m().log.level = raw->level;
        m().log.facility.arg = (void *)raw->facility;
        m().log.facility.funcs.encode = pb_encode_string;
        m().log.message.arg = (void *)raw->message;
        m().log.message.funcs.encode = pb_encode_string;
    }

};

class DataRecordMetadataMessage : public DataRecordMessage {
private:
    pb_data_t deviceIdData;
    fk_data_SensorInfo sensors[MaximumNumberOfSensors];
    pb_array_t sensorsArray;
    fk_data_ModuleInfo modules[MaximumNumberOfModules];
    pb_array_t modulesArray;

public:
    DataRecordMetadataMessage(CoreState &state, Pool &pool);

};

class DataRecordStatusMessage : public DataRecordMessage {
public:
    DataRecordStatusMessage(CoreState &state, Pool &pool);

};

}

#endif
