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

#include <Arduino.h>

namespace fk {

class Booting;
class Initializing;
class CheckPower;
class ScanAttachedDevices;
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

class Booting : public CoreDevice {
public:
    const char *name() const override {
        return "Booting";
    }

public:
    void task() override {
        transit<Initializing>();
    }
};

class Initializing : public CoreDevice {
public:
    const char *name() const override {
        return "Initializing";
    }

public:
    void task() override {
        transit_into<CheckPower>();
    }
};

class LowPowerSleep : public MainServicesState {
public:
    const char *name() const override {
        return "LowPowerSleep";
    }

public:
    void task() override {
        auto percentage = services().power->percentage();
        if (percentage > BatteryLowPowerResumeThreshold) {
            log("Battery: %f", percentage);
            transit_into<RebootDevice>();
        }
        else {
            transit_into<Sleep>(LowPowerSleepDurationSeconds);
        }
    }
};

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

class ScanAttachedDevices : public MainServicesState {
public:
    const char *name() const override {
        return "ScanAttachedDevices";
    }

public:
    void task() override {
        #if !defined(FK_NATURALIST)

        uint8_t addresses[4]{ 7, 8, 9, 0 };

        AttachedDevices attachedDevices{
            *services().state,
            *services().leds,
            *services().moduleCommunications,
            addresses,
        };

        while (simple_task_run(attachedDevices)) {
            // TODO: Should never take so long we need the watchdog.
            services().leds->task();
            services().moduleCommunications->task();
        }

        #ifdef FK_CORE_REQUIRE_MODULES
        if (services().state->numberOfModules(fk_module_ModuleType_SENSOR) == 0) {
            log("No attached modules.");
            transit<NoModulesThrottle>();
            return;
        }
        #endif

        #endif

        transit<WifiStartup>();
    }
};

class Sleep : public MainServicesState {
private:
    uint32_t maximum_{ 0 };
    uint32_t activity_{ 0 };

public:
    Sleep() {
    }

    Sleep(uint32_t maximum) : maximum_(maximum) {
    }

public:
    const char *name() const override {
        return "Sleep";
    }

public:
    void entry() override {
        MainServicesState::entry();
        activity_ = 0;

        DateTime wakeTime{ clock.getTime() + maximum_ };
        FormattedTime wakeFormatted{ wakeTime };
        log("Sleeping for %lu (%s)", maximum_, wakeFormatted.toString());
    }

    void react(UserButtonEvent const &ignored) override {
        activity_ = fk_uptime();
    }

    void task() override {
        auto started = fk_uptime();
        auto stopping = started + (maximum_ * 1000);

        services().fileSystem->flush();

        while (fk_uptime() < stopping) {
            auto delayed = false;

            if (activity_ == 0 || fk_uptime() - activity_ > 10000) {
                if (!fk_console_attached()) {
                    auto left = stopping - fk_uptime();
                    if (left > SleepMaximumGranularity) {
                        services().watchdog->sleep(SleepMaximumGranularity);
                        delayed = true;
                    }
                }

                if (!delayed) {
                    delay(1000);
                }
            }
            else {
                delay(10);
            }

            services().alive();

            // Should never be scheduled events during this.
            if (transitioned()) {
                return;
            }
        }

        back();
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

void CheckPower::task() {
    auto percentage = services().power->percentage();
    if (percentage < BatteryLowPowerSleepThreshold) {
        if (!fk_console_attached()) {
            transit<LowPowerSleep>();
            return;
        }
        else {
            log("Console attached, ignoring.");
        }
    }

    if (visited_) {
        back();
        return;
    }

    visited_ = true;

    transit<ScanAttachedDevices>();
}

void UserWakeup::task() {
    services().fileSystem->flush();

    transit<WifiStartup>();
}

void RebootDevice::task() {
    log("Rebooting!");

    services().fileSystem->flush();

    if (fk_console_attached()) {
        transit<Initializing>();
    }
    else {
        NVIC_SystemReset();
    }
}

void Idle::entry() {
    MainServicesState::entry();
    began_ = fk_uptime();

    auto now = clock.now();
    auto nextTask = services().scheduler->nextTask();
    DateTime runsAgain{ nextTask.time };
    FormattedTime nowFormatted{ now };
    FormattedTime runsAgainFormatted{ runsAgain };
    log("Idling (now = %s) (again = %s)", nowFormatted.toString(), runsAgainFormatted.toString());
}

void Idle::react(LowPowerEvent const &lpe) {
    if (fk_console_attached()) {
        log("Console attached, ignoring LowPowerEvent.");
    }
    else {
        transit<LowPowerSleep>();
    }
}

void Idle::react(SchedulerEvent const &se) {
    if (se.deferred) {
        warn("Scheduler Event!");
        transit(se.deferred);
    }
}

void Idle::task() {
    services().alive();

    if (fk_uptime() - checked_ > 250) {
        auto now = clock.now();

        // NOTE: Do this first to avoid a race condition. getNextTask doesn't
        // know that we didn't run a task yet, so it'll immediately return the
        // following task even if we're supposed to have run a sooner one.
        auto triggered = services().scheduler->check(lwcron::DateTime{ now.unixtime() });
        if (triggered) {
            triggered.task->run();
            return;
        }

        if (fk_uptime() - began_ > 60 * 1000) {
            auto nextTask = services().scheduler->nextTask();
            auto seconds = nextTask.time - now.unixtime();

            if (seconds > 70) {
                DateTime runsAgain{ nextTask.time };
                FormattedTime nowFormatted{ now };
                FormattedTime runsAgainFormatted{ runsAgain };
                log("Waiting (now = %s) (again = %s)", nowFormatted.toString(), runsAgainFormatted.toString());

                transit_into<Sleep>(seconds - 65);
                return;
            }
        }

        checked_ = fk_uptime();
    }
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
