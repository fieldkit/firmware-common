#ifndef FK_TWO_WIRE_TASK_H_INCLUDED
#define FK_TWO_WIRE_TASK_H_INCLUDED

#include "rtc.h"
#include "active_object.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "pool.h"

namespace fk {

class TwoWireTask : public Task {
private:
    TwoWireBus *bus;
    uint8_t address{ 0 };
    uint32_t dieAt{ 0 };
    uint32_t checkAt{ 0 };

protected:
    ModuleQueryMessage query;
    ModuleReplyMessage reply;

public:
    TwoWireTask(const char *name, TwoWireBus &bus, Pool &pool, uint8_t address)
        : Task(name), bus(&bus), address(address), query(pool), reply(pool) {
    }

    void enqueued() override {
        dieAt = 0;
        checkAt = 0;
    }

    ModuleReplyMessage &replyMessage() {
        return reply;
    }

    TaskEval task() override;

};

class QueryCapabilities : public TwoWireTask {
    static constexpr char Name[] = "QueryCapabilities";

public:
    QueryCapabilities(TwoWireBus &bus, Pool &pool, uint8_t address) : TwoWireTask(Name, bus, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_CAPABILITIES;
        query.m().queryCapabilities.version = FK_MODULE_PROTOCOL_VERSION;
    }

    void enqueued() override {
        TwoWireTask::enqueued();
        query.m().queryCapabilities.callerTime = clock.getTime();
    }

    size_t numberOfSensors() {
        return reply.m().capabilities.numberOfSensors;
    }

};

class QuerySensorCapabilities : public TwoWireTask {
    static constexpr char Name[] = "QuerySensorCapabilities";

public:
    QuerySensorCapabilities(TwoWireBus &bus, Pool &pool, uint8_t address, uint8_t sensor) : TwoWireTask(Name, bus, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_SENSOR_CAPABILITIES;
        query.m().querySensorCapabilities.sensor = sensor;
    }

    uint8_t sensor() {
        return query.m().querySensorCapabilities.sensor;
    }

    void done() override {
        log("Sensor #%lu: '%s'", reply.m().sensorCapabilities.id, (const char *)reply.m().sensorCapabilities.name.arg);
    }

};

class BeginTakeReading : public TwoWireTask {
    static constexpr char Name[] = "BeginTakeReading";

public:
    BeginTakeReading(TwoWireBus &bus, Pool &pool, uint8_t address) : TwoWireTask(Name, bus, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS;
        query.m().beginTakeReadings.index = 0;
    }

public:
    void enqueued() override {
        TwoWireTask::enqueued();
        query.m().beginTakeReadings.callerTime = clock.getTime();
    }

    uint32_t getBackoff() {
            return reply.m().readingStatus.backoff;
    }

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
    QueryReadingStatus(TwoWireBus &bus, Pool &pool, uint8_t address) : TwoWireTask(Name, bus, pool, address) {
        query.m().type = fk_module_QueryType_QUERY_READING_STATUS;
    }

public:
    uint32_t getBackoff() {
        return reply.m().readingStatus.backoff;
    }

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
