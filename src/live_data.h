#ifndef FK_LIVE_DATA_H_INCLUDED
#define FK_LIVE_DATA_H_INCLUDED

#include "active_object.h"
#include "core_state.h"
#include "gather_readings.h"

namespace fk {

class LiveData : public ActiveObject {
private:
    Task *gatherReadings;
    CoreState *state;
    uint32_t lastRead{ 0 };
    uint32_t interval{ 0 };

public:
    LiveData(Task &gatherReadings, CoreState &state);

public:
    void start(uint32_t newInterval);
    void stop();
    void done(Task &task) override;
    void error(Task &task) override;

};

}

#endif
