#ifndef FK_CORE_MODULE_H_INCLUDED
#define FK_CORE_MODULE_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

#include "pool.h"
#include "two_wire_task.h"

namespace fk {

class GatherReadings : public Task {
private:
    CoreState *state;
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;

public:
    GatherReadings(CoreState &state, Pool &pool);

public:
    TaskEval task() override;

};

class SendTransmission : public Task {
private:
    CoreState *state;

public:
    SendTransmission(CoreState &state, Pool &pool);

public:
    TaskEval task() override;

};

class SendStatus : public Task {
private:
    CoreState *state;

public:
    SendStatus(CoreState &state, Pool &pool);

public:
    TaskEval task() override;

};

class DetermineLocation : public Task {
private:
    CoreState *state;

public:
    DetermineLocation(CoreState &state, Pool &pool);

public:
    TaskEval task() override;

};

class Core : public ActiveObject {
private:

public:
    Core();

public:

};

}

#endif
