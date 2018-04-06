#ifndef FK_GATHER_READINGS_H_INCLUDED
#define FK_GATHER_READINGS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"
#include "module_comms.h"

namespace fk {

class Leds;

class BeginTakeReading : public ModuleQuery {
private:
    uint32_t backoff{ 0 };
    uint32_t status{ 0 };

public:
    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_BEGIN_TAKE_READINGS;
        message.m().beginTakeReadings.callerTime = clock.getTime();
        message.m().beginTakeReadings.index = 0;
    }

    void reply(ModuleReplyMessage &message) override {
        backoff = message.m().readingStatus.backoff;
        status = message.m().readingStatus.state;
    }

    uint32_t getBackoff() {
        return backoff;
    }

    bool isBegin() {
        return status == fk_module_ReadingState_BEGIN;
    }

    bool isDone() {
        return status == fk_module_ReadingState_DONE;
    }

    bool isBusy() {
        return status == fk_module_ReadingState_BUSY;
    }

    bool isIdle() {
        return status == fk_module_ReadingState_IDLE;
    }

};

class QueryReadingStatus : public ModuleQuery {
private:
    uint32_t backoff{ 0 };
    uint32_t status{ 0 };

public:
    void query(ModuleQueryMessage &message) override {
        message.m().type = fk_module_QueryType_QUERY_READING_STATUS;
    }

    void reply(ModuleReplyMessage &message) override {
        backoff = message.m().readingStatus.backoff;
        status = message.m().readingStatus.state;
    }

    uint32_t getBackoff() {
        return backoff;
    }

    bool isBegin() {
        return status == fk_module_ReadingState_BEGIN;
    }

    bool isDone() {
        return status == fk_module_ReadingState_DONE;
    }

    bool isBusy() {
        return status == fk_module_ReadingState_BUSY;
    }

    bool isIdle() {
        return status == fk_module_ReadingState_IDLE;
    }

};

class GatherReadings : public Task {
private:
    CoreState *state;
    Leds *leds;
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;
    ModuleProtocolHandler protocol;
    uint8_t retries{ 0 };

public:
    GatherReadings(TwoWireBus &bus, CoreState &state, Leds &leds, ModuleCommunications &communications, Pool &pool);

public:
    void enqueued() override;
    TaskEval task() override;
    TaskEval done(ModuleProtocolHandler::Finished &finished);
    TaskEval error(ModuleProtocolHandler::Finished &finished);
    void error() override;
    void done() override;

};

}

#endif
