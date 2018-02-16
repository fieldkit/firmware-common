#ifndef FK_GATHER_READINGS_H_INCLUDED
#define FK_GATHER_READINGS_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "two_wire_task.h"

namespace fk {

class Leds;

class GatherReadings : public ActiveObject {
private:
    CoreState *state;
    Leds *leds;
    Delay delay{ 300 };
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;
    uint8_t retries{ 0 };

public:
    GatherReadings(TwoWireBus &bus, CoreState &state, Leds &leds, Pool &pool);

public:
    void enqueued() override;
    void done(Task &task) override;
    void error(Task &task) override;
    void error() override;
    void done() override;

};

}

#endif
