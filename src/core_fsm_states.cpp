#include <Arduino.h>

#include "core_fsm_states.h"
#include "wifi_states.h"

#include "attached_devices.h"
#include "scheduler.h"
#include "leds.h"
#include "watchdog.h"
#include "status.h"
#include "power_management.h"
#include "user_button.h"

#include "gather_readings.h"

#include "file_system.h"
#include "wifi_states.h"
#include "gps.h"

#include "scan_attached_deviecs.h"
#include "check_power.h"
#include "low_power_sleep.h"

namespace fk {

class Sleep;

template<>
MainServices *MainServicesState::services_{ nullptr };

template<>
WifiServices *WifiServicesState::services_{ nullptr };

void MainServices::alive() {
    leds->task();
    watchdog->task();
    power->task();
    status->task();
    gps->task();
    button->task();
}

DateTime MainServices::scheduledTasks() {
    auto now = clock.now();
    auto lwcTime = lwcron::DateTime{ now.unixtime() };
    scheduler->check(lwcTime);
    return now;
}

void MainServices::clear() {
    pool->clear();
}

void Initialized::task() {
    transit_into<CheckPower>();
}

class NoModulesThrottle : public MainServicesState {
private:
    uint32_t entered_{ 0 };

public:
    const char *name() const override {
        return "NoModulesThrottle";
    }

public:
    void entry() override {
        MainServicesState::entry();
        entered_ = fk_uptime();
    }

    void task() override {
        services().alive();

        if (fk_uptime() - entered_ > NoModulesRebootWait) {
            transit<RebootDevice>();
        }
    }
};

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

void UserWakeup::task() {
    services().fileSystem->flush();

    transit<WifiStartup>();
}

void RebootDevice::task() {
    log("Rebooting!");

    services().fileSystem->flush();

    if (fk_console_attached()) {
        transit<Initialized>();
    }
    else {
        NVIC_SystemReset();
    }
}

}
