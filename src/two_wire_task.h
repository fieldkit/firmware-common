#ifndef FK_TWO_WIRE_TASK_H_INCLUDED
#define FK_TWO_WIRE_TASK_H_INCLUDED

#include "rtc.h"
#include "active_object.h"
#include "message_buffer.h"
#include "module_messages.h"
#include "pool.h"
#include "streams.h"

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

class StreamTwoWireTask : public Task {
private:
    TwoWireBus *bus;
    Reader *outgoing;
    Writer *incoming;
    uint8_t address{ 0 };
    uint32_t dieAt{ 0 };
    uint32_t checkAt{ 0 };
    uint32_t doneAt{ 0 };
    size_t bytesReceived{ 0 };

public:
    /**
     *
     */
    StreamTwoWireTask(const char *name, TwoWireBus &bus, Reader &outgoing, Writer &incoming, uint8_t address);

    /**
     *
     */
    StreamTwoWireTask(const char *name, TwoWireBus &bus, Writer &incoming, uint8_t address);

public:
    void enqueued() override;

    TaskEval task() override;

    void error() override {
        doneAt = millis();
    }

    void done() override {
        doneAt = millis();
    }

    bool completed() {
        return doneAt > 0;
    }

    size_t received() {
        return bytesReceived;
    }

private:
    TaskEval send();

    TaskEval receive();

};

class BeginTakeReading : public TwoWireTask {
public:
    BeginTakeReading(TwoWireBus &bus, Pool &pool, uint8_t address) : TwoWireTask("BeginTakeReading", bus, pool, address) {
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
public:
    QueryReadingStatus(TwoWireBus &bus, Pool &pool, uint8_t address) : TwoWireTask("QueryReadingStatus", bus, pool, address) {
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

class CustomModuleQueryTask : public TwoWireTask {
private:
    AppQueryMessage *appQuery;
    Pool *pool;
    pb_data_t messageData;

public:
    CustomModuleQueryTask(TwoWireBus &bus, AppQueryMessage &appQuery, Pool &pool, uint8_t address) : TwoWireTask("CustomModuleQuery", bus, pool, address), appQuery(&appQuery), pool(&pool) {
        query.m().type = fk_module_QueryType_QUERY_CUSTOM;
    }

    void enqueued() override {
        TwoWireTask::enqueued();
        query.m().custom.message.arg = appQuery->m().module.message.arg;
    }

};

}

#include "module_comms.h"

#endif
