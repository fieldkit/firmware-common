#ifndef FK_TWO_WIRE_TASK_H_INCLUDED
#define FK_TWO_WIRE_TASK_H_INCLUDED

#include "active_object.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "pool.h"

namespace fk {

class TwoWireTask : public Task {
protected:
    ModuleQueryMessage query;
    ModuleReplyMessage reply;

private:
    uint8_t address{ 0 };
    uint32_t dieAt{ 0 };
    uint32_t checkAt{ 0 };

public:
    TwoWireTask(const char *name, Pool *pool, uint8_t address)
        : Task(name), query(pool), reply(pool), address(address) {
    }

    void enqueued() override {
        dieAt = 0;
        checkAt = 0;
    }

    TaskEval task() override;

};

class QueryCapabilities : public TwoWireTask {
    static constexpr char Name[] = "QueryCapabilities";

public:
    QueryCapabilities(Pool *pool, uint8_t address, uint32_t now) : TwoWireTask(Name, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_CAPABILITIES;
        query.m().queryCapabilities.version = FK_MODULE_PROTOCOL_VERSION;
        query.m().queryCapabilities.callerTime = 0;
    }

    size_t numberOfSensors() {
        return reply.m().capabilities.numberOfSensors;
    }

};

class QuerySensorCapabilities : public TwoWireTask {
    static constexpr char Name[] = "QuerySensorCapabilities";

public:
    QuerySensorCapabilities(Pool *pool, uint8_t address, uint8_t sensor) : TwoWireTask(Name, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_SENSOR_CAPABILITIES;
        query.m().querySensorCapabilities.sensor = sensor;
    }

    uint8_t sensor() {
        return query.m().querySensorCapabilities.sensor;
    }

    void done() override {
        log("Sensor #%d: '%s'", reply.m().sensorCapabilities.id, reply.m().sensorCapabilities.name.arg);
    }

};

class BeginTakeReading : public TwoWireTask {
    static constexpr char Name[] = "BeginTakeReading";

public:
    BeginTakeReading(Pool *pool, uint8_t address) : TwoWireTask(Name, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS;
    }

public:
    bool isDone() {
        return reply.m().readingStatus.state == fk_module_ReadingState_DONE;
    }

    bool isBusy() {
        return reply.m().readingStatus.state == fk_module_ReadingState_BUSY;
    }

    bool isIdle() {
        return reply.m().readingStatus.state == fk_module_ReadingState_IDLE;
    }

};

class QueryReadingStatus : public TwoWireTask {
    static constexpr char Name[] = "QueryReadingStatus";

public:
    QueryReadingStatus(Pool *pool, uint8_t address) : TwoWireTask(Name, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_READING_STATUS;
    }

public:
    bool isDone() {
        return reply.m().readingStatus.state == fk_module_ReadingState_DONE;
    }

    bool isBusy() {
        return reply.m().readingStatus.state == fk_module_ReadingState_BUSY;
    }

    bool isIdle() {
        return reply.m().readingStatus.state == fk_module_ReadingState_IDLE;
    }

};

}

#endif
