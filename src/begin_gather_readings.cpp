#include "begin_gather_readings.h"

#include "gather_readings.h"
#include "file_system.h"
#include "gps.h"

namespace fk {

class TakeReadings : public MainServicesState {
private:
    uint8_t remaining_{ 1 };

public:
    const char *name() const override {
        return "TakeReadings";
    }

public:
    void entry() override {
        MainServicesState::entry();
        remaining_ = services().state->readingsToTake();
    }

    void task() override {
        while (remaining_ > 0) {
            GatherReadings gatherReadings{
                remaining_,
                *services().state,
                *services().leds,
                *services().moduleCommunications
            };

            gatherReadings.enqueued();

            log("Taking reading %d", remaining_);
            while (simple_task_run(gatherReadings)) {
                services().alive();
                services().moduleCommunications->task();
            }
            remaining_--;
        }

        services().fileSystem->flush();

        resume();
    }
};

class TakeGpsReading : public MainServicesState {
private:
    uint32_t interval_{ GpsFixAttemptInterval };

public:
    TakeGpsReading() {
    }

    TakeGpsReading(uint32_t interval) : interval_(interval) {
    }

public:
    const char *name() const override {
        return "TakeGpsReading";
    }

public:
    void task() override {
        services().gps->save();

        transit<TakeReadings>();
    }
};

void BeginGatherReadings::task() {
    resume_at_back();

    transit<TakeGpsReading>();
}

}
