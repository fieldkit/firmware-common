#ifndef FK_CORE_MODULE_PARTS_H_INCLUDED
#define FK_CORE_MODULE_PARTS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

#include "pool.h"
#include "two_wire_task.h"
#include "transmissions.h"

namespace fk {

class GatherReadings : public ActiveObject {
private:
    CoreState *state;
    Delay delay{ 300 };
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;

public:
    GatherReadings(CoreState &state, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

class SendTransmission : public ActiveObject {
private:
    MessageBuilder *builder;
    TransmissionTask *method;

public:
    SendTransmission(MessageBuilder &builder, TransmissionTask &method, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

class SendStatus : public ActiveObject {
private:
    MessageBuilder *builder;
    TransmissionTask *method;

public:
    SendStatus(MessageBuilder &builder, TransmissionTask &method, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

class DetermineLocation : public ActiveObject {
private:
    CoreState *state;

public:
    DetermineLocation(CoreState &state, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;

};

}

#endif
