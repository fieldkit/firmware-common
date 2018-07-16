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
    button->task();
    power->task();
    status->task();
    gps->task();
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
            transit_into<Sleep>((uint32_t)60);
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

        #endif

        transit<WifiStartup>();
    }
};

class Sleep : public MainServicesState {
private:
    uint32_t maximum_{ 0 };

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
    void task() override {
        log("Maximum: %lu", maximum_);

        auto started = fk_uptime();

        services().fileSystem->flush();

        while (fk_uptime() - started < maximum_ * 1000) {
            auto delayed = false;

            if (!fk_console_attached()) {
                if (fk_uptime() - started > SleepMaximumGranularity) {
                    services().leds->all(true);
                    delay(100);
                    services().leds->all(false);

                    // TODO: Power down peripherals?
                    services().watchdog->sleep(SleepMaximumGranularity);
                    delayed = true;
                }
            }

            if (!delayed) {
                delay(1000);
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
public:
    const char *name() const override {
        return "TakeReadings";
    }

public:
    void task() override {
        GatherReadings gatherReadings{
            *services().state,
            *services().leds,
            *services().moduleCommunications
        };

        gatherReadings.enqueued();

        while (simple_task_run(gatherReadings)) {
            services().alive();
            services().moduleCommunications->task();
        }

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
    if (began_ == 0) {
        began_ = fk_uptime();
    }
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
    if (fk_uptime() - checked_ > 500) {
        auto nextTask = services().scheduler->getNextTask();
        if (nextTask.seconds > 10) {
            transit_into<Sleep>(nextTask.seconds - 5);
            return;
        }
        checked_ = fk_uptime();
    }

    services().scheduler->task();

    services().alive();
}

}

FSM_INITIAL_STATE(fk::CoreDevice, fk::Booting)
