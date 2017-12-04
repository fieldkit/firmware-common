#ifndef FK_LIVE_DATA_H_INCLUDED
#define FK_LIVE_DATA_H_INCLUDED

#include "active_object.h"
#include "core_state.h"

namespace fk {

class LiveData : public ActiveObject {
private:
    uint32_t interval{ 0 };
    CoreState *state;
    Pool *pool;

private:
    Delay checkDelay;
    Delay takeReadingsDelay;
    BeginTakeReading beginTakeReading;
    QueryReadingStatus queryReadingStatus;

public:
    LiveData(CoreState &state, Pool &pool);

public:
    void start(uint32_t newInterval);
    void stop();
    bool hasReadings();
    void done(Task &task) override;
    void error(Task &task) override;

};

}

#endif
